#define STB_IMAGE_IMPLEMENTATION
#include "cmx_texture.h"

// cmx
#include "cmx_buffer.h"
#include "cmx_device.h"
#include "cmx_model.h"
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

size_t Texture::_boundID{MAX_SAMPLER_SETS + 1};

void Texture::resetBoundID()
{
    _boundID = {MAX_SAMPLER_SETS};
}

Texture::Texture(Device *device, const Texture::Builder &builder, const std::string &name) : name{name}
{
    createImage(device, builder);
    createImageView(device, builder);
    generateMipmaps(device, builder);
    createSampler(device);
    _renderSystem = RenderSystem::getInstance();
    _descriptorSetID = _renderSystem->createSamplerDescriptor(_imageView, _sampler);
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
    Device *device = _renderSystem->getDevice();

    if (device == nullptr)
    {
        spdlog::error("Texture: cannot free texture after having destroyed device");
        std::exit(EXIT_FAILURE);
    }

    device->device().destroySampler(_sampler);
    device->device().destroyImage(_image);
    device->device().destroyImageView(_imageView);
    device->device().freeMemory(_imageMemory);
    _renderSystem->freeSamplerDescriptor(_descriptorSetID);

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

    vk::ImageCreateInfo imageInfo{};

    imageInfo.sType = vk::StructureType::eImageCreateInfo;
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.extent.width = builder.width;
    imageInfo.extent.height = builder.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = builder.mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = vk::Format::eR8G8B8A8Unorm;
    imageInfo.tiling = vk::ImageTiling::eOptimal;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageInfo.usage =
        vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;

    device->createImageWithInfo(imageInfo, {vk::MemoryPropertyFlagBits::eDeviceLocal}, _image, _imageMemory);

    // Copy image data
    device->transitionImageLayout(_image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
                                  builder.mipLevels);
    device->copyBufferToImage(imageStagingBuffer.getBuffer(), _image, builder.width, builder.height, 1);

    imageStagingBuffer.free();
}

void Texture::Builder::loadTexture(const std::string &filepath)
{
    int channels;

    image = stbi_load(filepath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

    if (!image)
    {
        spdlog::info("stbiload: failed to load texture file {0}", filepath.c_str());
        std::exit(EXIT_FAILURE);
    }

    this->filepath = filepath;
    imageSize = width * height * 4;
    format = vk::Format::eR8G8B8A8Unorm;
}

void Texture::createImageView(Device *device, const Builder &builder)
{
    vk::ImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.image = _image;
    imageViewCreateInfo.sType = vk::StructureType::eImageViewCreateInfo;
    imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
    imageViewCreateInfo.format = builder.format;
    imageViewCreateInfo.components.r = vk::ComponentSwizzle::eIdentity;
    imageViewCreateInfo.components.g = vk::ComponentSwizzle::eIdentity;
    imageViewCreateInfo.components.b = vk::ComponentSwizzle::eIdentity;
    imageViewCreateInfo.components.a = vk::ComponentSwizzle::eIdentity;
    imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = 1;
    imageViewCreateInfo.subresourceRange.levelCount = builder.mipLevels;

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
    samplerCreateInfo.maxLod = 10.0f;
    samplerCreateInfo.anisotropyEnable = true;
    samplerCreateInfo.maxAnisotropy = 16;

    _sampler = device->device().createSampler(samplerCreateInfo);
}

void Texture::generateMipmaps(class Device *device, const Builder &builder)
{
    vk::FormatProperties formatProperties = device->physicalDevice().getFormatProperties(builder.format);
    if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
    {
        throw std::runtime_error("texture image format does not support linear blitting!");
    }

    vk::CommandBuffer commandBuffer = device->beginSingleTimeCommands();

    vk::ImageMemoryBarrier barrier{};
    barrier.image = _image;
    barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;
    int32_t mipWidth = builder.width;
    int32_t mipHeight = builder.height;

    for (uint32_t i = 1; i < builder.mipLevels; i++)
    {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, 0,
                                      nullptr, 0, nullptr, 1, &barrier);

        vk::ImageBlit blit{};
        blit.srcOffsets[0] = vk::Offset3D{0, 0, 0};
        blit.srcOffsets[1] = vk::Offset3D{mipWidth, mipHeight, 1};
        blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;

        blit.dstOffsets[0] = vk::Offset3D{0, 0, 0};
        blit.dstOffsets[1] = vk::Offset3D{mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
        blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        commandBuffer.blitImage(_image, vk::ImageLayout::eTransferSrcOptimal, _image,
                                vk::ImageLayout::eTransferDstOptimal, 1, &blit, vk::Filter::eLinear);

        barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
                                      {}, 0, nullptr, 0, nullptr, 1, &barrier);

        if (mipWidth > 1)
            mipWidth /= 2;
        if (mipHeight > 1)
            mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = builder.mipLevels - 1;
    barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {},
                                  0, nullptr, 0, nullptr, 1, &barrier);

    device->endSingleTimeCommands(commandBuffer);
}

void Texture::bind(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout)
{
    if (_boundID != _descriptorSetID)
    {
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 1, 1,
                                         &(_renderSystem->getSamplerDescriptorSet(_descriptorSetID)), 0, nullptr);

        _boundID = _descriptorSetID;
    }
}

} // namespace cmx
