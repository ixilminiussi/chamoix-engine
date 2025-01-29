#define STB_IMAGE_IMPLEMENTATION
#include "cmx_texture.h"

// cmx
#include "cmx_buffer.h"
#include "cmx_device.h"
#include "cmx_render_system.h"

// lib
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

// std
#include <cstdlib>

namespace cmx
{

Texture::Texture(Device *device, const Texture::Builder &builder, const std::string &name) : name{name}
{
    createImage(device, builder);
    createImageView(device, _image);
    RenderSystem::createTextureDescriptor(_imageView, _sampler);
    _filepath = builder.filepath;
}

Texture::~Texture()
{
    if (!_freed)
    {
        spdlog::error("Texture: forgot to free before deletion!");
    }
}

void Texture::free()
{
    Device *device = RenderSystem::getDevice();

    if (device == nullptr)
    {
        spdlog::error("Texture: cannot free texture after having destroyed device");
        std::exit(EXIT_FAILURE);
    }

    device->device().destroySampler(_sampler);
    device->device().destroyImage(_image);
    device->device().destroyImageView(_imageView);
    device->device().freeMemory(_imageMemory);

    _freed = true;
}

Texture *Texture::createTextureFromFile(class Device *device, const std::string &filepath, const std::string &name)
{
    Builder builder{};
    builder.loadTexture(filepath);

    spdlog::info("Texture: '{0}' loaded with resolution {1}x{2}", filepath, builder.width, builder.height);
    return new Texture(device, builder, name);
}

void Texture::createImage(Device *device, const Builder &builder)
{
    // Create staging buffer to hold loaded data, ready to copy to device
    Buffer imageStagingBuffer{*device, builder.imageSize, 1, vk::BufferUsageFlagBits::eTransferSrc,
                              vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent};

    imageStagingBuffer.map(builder.imageSize, 0);
    imageStagingBuffer.writeToBuffer(builder.image);
    imageStagingBuffer.unmap();

    // Free original image data
    stbi_image_free(builder.image);

    uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(builder.width, builder.height)))) + 1;

    vk::ImageCreateInfo imageInfo{};

    imageInfo.sType = vk::StructureType::eImageCreateInfo;
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.extent.width = builder.width;
    imageInfo.extent.height = builder.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = vk::Format::eR8G8B8A8Unorm;
    imageInfo.tiling = vk::ImageTiling::eOptimal;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageInfo.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;

    device->createImageWithInfo(imageInfo, {vk::MemoryPropertyFlagBits::eDeviceLocal}, _image, _imageMemory);

    // Copy image data
    device->transitionImageLayout(_image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    device->copyBufferToImage(imageStagingBuffer.getBuffer(), _image, builder.width, builder.height, 1);
    device->transitionImageLayout(_image, vk::ImageLayout::eTransferDstOptimal,
                                  vk::ImageLayout::eShaderReadOnlyOptimal);

    imageStagingBuffer.free();
}

void Texture::Builder::loadTexture(const std::string &filepath)
{
    int channels;

    image = stbi_load(filepath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (!image)
    {
        spdlog::info("stbiload: failed to load texture file {0}", filepath.c_str());
        std::exit(EXIT_FAILURE);
    }

    this->filepath = filepath;
    imageSize = width * height * 4;
}

void Texture::createImageView(Device *device, const vk::Image &)
{
    vk::ImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.image = _image;
    imageViewCreateInfo.sType = vk::StructureType::eImageViewCreateInfo;
    imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
    imageViewCreateInfo.format = vk::Format::eR8G8B8A8Unorm;
    imageViewCreateInfo.components.r = vk::ComponentSwizzle::eIdentity;
    imageViewCreateInfo.components.g = vk::ComponentSwizzle::eIdentity;
    imageViewCreateInfo.components.b = vk::ComponentSwizzle::eIdentity;
    imageViewCreateInfo.components.a = vk::ComponentSwizzle::eIdentity;
    imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = 1;

    _imageView = device->device().createImageView(imageViewCreateInfo);
}

tinyxml2::XMLElement &Texture::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement)
{
    tinyxml2::XMLElement *textureElement = doc.NewElement("texture");

    textureElement->SetAttribute("filepath", _filepath.c_str());
    parentElement->InsertEndChild(textureElement);

    return *textureElement;
}

void Texture::createSampler(class Device *device)
{
    vk::SamplerCreateInfo samplerCreateInfo{};
    samplerCreateInfo.magFilter = vk::Filter::eLinear;
    samplerCreateInfo.minFilter = vk::Filter::eLinear;
    samplerCreateInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
    samplerCreateInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
    samplerCreateInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
    samplerCreateInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
    samplerCreateInfo.unnormalizedCoordinates = false;
    samplerCreateInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
    samplerCreateInfo.mipLodBias = 0.0f;
    samplerCreateInfo.minLod = 0.0f;
    samplerCreateInfo.maxLod = 0.0f;
    samplerCreateInfo.anisotropyEnable = true;
    samplerCreateInfo.maxAnisotropy = 16;

    _sampler = device->device().createSampler(samplerCreateInfo);
}

} // namespace cmx
