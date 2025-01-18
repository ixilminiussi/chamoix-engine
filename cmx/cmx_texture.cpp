#include "cmx_texture.h"

// cmx
#include "cmx_buffer.h"
#include "cmx_render_system.h"

// lib
#include <spdlog/spdlog.h>
#include <stb_image.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

// std
#include <cstdlib>

namespace cmx
{

CmxTexture::CmxTexture(CmxDevice *device, const CmxTexture::Builder &builder, const std::string &name)
    : name{name}, _filepath{builder.filepath}
{
    if (!device)
        return;

    _stagingBuffer =
        std::make_unique<CmxBuffer>(*device, builder.imageSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
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

    if (vkCreateImage(device->device(), &imageInfo, nullptr, &textureImage) != VK_SUCCESS)
    {
        spdlog::error("CmxTexture: failed to create image!");
        std::exit(EXIT_FAILURE);
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device->device(), textureImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    VkPhysicalDeviceMemoryProperties memProperties;

    // find memory type
    vkGetPhysicalDeviceMemoryProperties(device->physicalDevice(), &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((memRequirements.memoryTypeBits & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) ==
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        {
            allocInfo.memoryTypeIndex = i;
            break;
        }
    }

    if (vkAllocateMemory(device->device(), &allocInfo, nullptr, &textureImageMemory) != VK_SUCCESS)
    {
        spdlog::error("CmxTexture: failed to allocate image memory!");
        std::exit(EXIT_FAILURE);
    }

    vkBindImageMemory(device->device(), textureImage, textureImageMemory, 0);

    stbi_image_free(builder.pixels);
}

CmxTexture::~CmxTexture()
{
    if (!_freed)
    {
        spdlog::error("CmxTexture: forgot to free texture {0} before deletion", name);
    }
}

CmxTexture *CmxTexture::createTextureFromFile(CmxDevice *device, const std::string &filepath, const std::string &name)
{
    Builder builder{};
    builder.loadTexture(filepath);

    spdlog::info("CmxTexture: '{0}' loaded", filepath);
    return new CmxTexture(device, builder, name);
}

void CmxTexture::bind(VkCommandBuffer)
{
}

void CmxTexture::free()
{
    delete _stagingBuffer.release();

    CmxDevice *device = RenderSystem::getDevice();
    if (device)
    {
        vkDestroyImage(device->device(), textureImage, nullptr);
        vkFreeMemory(device->device(), textureImageMemory, nullptr);
    }
    else
    {
        spdlog::warn("CmxTexture: Cannot properly free texture without device, make sure device is being deleted last");
    }

    _freed = true;
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
