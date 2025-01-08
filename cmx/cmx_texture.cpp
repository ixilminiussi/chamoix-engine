#include "cmx_texture.h"
#include "cmx_buffer.h"

// lib
#include <cstdlib>
#include <spdlog/spdlog.h>
#include <stb_image.h>
#include <vulkan/vulkan.h>

// std
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace cmx
{

CmxTexture::CmxTexture(std::shared_ptr<class CmxDevice> cmxDevice, const CmxTexture::Builder &builder,
                       const std::string &name)
    : _cmxDevice{cmxDevice}, name{name}, _filepath{builder.filepath}
{
    _stagingBuffer = std::make_unique<CmxBuffer>(
        *_cmxDevice, builder.imageSize, static_cast<size_t>(builder.imageSize), VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    _stagingBuffer->map(builder.imageSize, 0);
    _stagingBuffer->writeToBuffer(builder.pixels, static_cast<size_t>(builder.imageSize), 0);
    _stagingBuffer->unmap();

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(builder.texWidth);
    imageInfo.extent.height = static_cast<uint32_t>(builder.texHeight);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    if (vkCreateImage(_cmxDevice->device(), &imageInfo, nullptr, &textureImage) != VK_SUCCESS)
    {
        spdlog::error("CmxTexture: failed to create image!");
        std::exit(EXIT_FAILURE);
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(_cmxDevice->device(), textureImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    VkPhysicalDeviceMemoryProperties memProperties;

    // find memory type
    vkGetPhysicalDeviceMemoryProperties(_cmxDevice->physicalDevice(), &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((memRequirements.memoryTypeBits & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) ==
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        {
            allocInfo.memoryTypeIndex = i;
        }
    }

    if (vkAllocateMemory(_cmxDevice->device(), &allocInfo, nullptr, &textureImageMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(_cmxDevice->device(), textureImage, textureImageMemory, 0);

    stbi_image_free(builder.pixels);
}

std::shared_ptr<CmxTexture> CmxTexture::createTextureFromFile(std::shared_ptr<class CmxDevice> device,
                                                              const std::string &filepath, const std::string &name)
{
    Builder builder{};
    builder.loadTexture(filepath);

    spdlog::info("CmxTexture: '{0}' loaded", filepath);
    return std::make_shared<CmxTexture>(device, builder, name);
}

void CmxTexture::bind(VkCommandBuffer)
{
}

void CmxTexture::Builder::loadTexture(const std::string &filepath)
{
    int texWidth, texHeight, texChannels;
    pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    imageSize = texWidth * texHeight * 4;

    if (!pixels)
    {
        spdlog::error("stbi: failed to load texture image!");
        std::exit(EXIT_FAILURE);
    }

    this->texWidth = (uint32_t)texWidth;
    this->texHeight = (uint32_t)texHeight;
    this->filepath = filepath;
}

tinyxml2::XMLElement &CmxTexture::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement)
{
    tinyxml2::XMLElement *textureElement = doc.NewElement("texture");

    textureElement->SetAttribute("filepath", _filepath.c_str());
    parentElement->InsertEndChild(textureElement);

    return *textureElement;
}

} // namespace cmx
