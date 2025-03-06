#include <stdexcept>
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
    _boundID = MAX_SAMPLER_SETS;
}

Texture::Texture(Device *device, const Texture::Builder &builder, const char *name) : name{name}
{
    _imageType = builder.depth > 1 ? vk::ImageType::e3D : vk::ImageType::e2D;

    createImage(device, builder);
    createImageView(device, builder);
    generateMipmaps(device, builder);
    createSampler(device);

    _renderSystem = RenderSystem::getInstance();
    _descriptorSetID = _renderSystem->createSamplerDescriptor(_imageView, _sampler);
    _filepaths = builder.filepaths;
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
        throw std::runtime_error("Texture: cannot free texture after having destroyed device");
    }

    device->device().destroySampler(_sampler);
    device->device().destroyImage(_image);
    device->device().destroyImageView(_imageView);
    device->device().freeMemory(_imageMemory);
    _renderSystem->freeSamplerDescriptor(_descriptorSetID);

    _freed = true;
}

Texture *Texture::create2DTextureFromFile(class Device *device, const char *filepath, const char *name)
{
    Builder builder{};
    builder.loadTexture(filepath);

    spdlog::info("Texture: '{0}', <{1}> loaded with resolution {2}x{3}", filepath, name, builder.width, builder.height);
    return new Texture(device, builder, name);
}

Texture *Texture::create3DTextureFromFile(class Device *device, const std::vector<std::string> &filepaths,
                                          const char *name)
{
    if (filepaths.size() <= 1)
    {
        throw std::runtime_error("AssetsManager: 3d textures MUST have more than one texture");
    }

    Builder builder{};
    builder.loadTexture3D(filepaths);

    spdlog::info("Texture: <{0}> loaded with resolution {1}x{2}x{3}", name, builder.width, builder.height,
                 filepaths.size());
    return new Texture(device, builder, name);
}

void Texture::createImage(Device *device, const Builder &builder)
{
    // Create staging buffer to hold loaded data, ready to copy to device
    Buffer imageStagingBuffer{*device, builder.imageSize, 1, vk::BufferUsageFlagBits::eTransferSrc,
                              vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent};

    imageStagingBuffer.map(builder.imageSize, 0);

    int sliceSize = builder.width * builder.height * 4;
    for (int i = 0; i < builder.depth; i++)
    {
        imageStagingBuffer.writeToBuffer(builder.images[i], sliceSize, i * sliceSize);
        stbi_image_free(builder.images[i]);
    }
    imageStagingBuffer.unmap();

    vk::ImageCreateInfo imageInfo{};

    imageInfo.sType = vk::StructureType::eImageCreateInfo;
    imageInfo.imageType = _imageType;
    imageInfo.extent.width = builder.width;
    imageInfo.extent.height = builder.height;
    imageInfo.extent.depth = builder.depth;
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
    device->copyBufferToImage(imageStagingBuffer.getBuffer(), _image, builder.width, builder.height, builder.depth);

    imageStagingBuffer.free();
}

void Texture::Builder::loadTexture(const char *filepath)
{
    int channels;

    images = {stbi_load(filepath, &width, &height, &channels, STBI_rgb_alpha)};

    mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

    if (!images[0])
    {
        throw std::runtime_error(std::string("Texture: stbiload failed to load texture file ") + filepath);
    }

    this->filepaths = {filepath};
    imageSize = width * height * 4;
    format = vk::Format::eR8G8B8A8Unorm;
}

void Texture::Builder::loadTexture3D(const std::vector<std::string> &filepaths)
{
    int channels;

    int numImages = filepaths.size();

    if (numImages == 0)
    {
        spdlog::error("Texture: cannot load 3d texture with 0 paths");
        std::exit(EXIT_FAILURE);
    }

    int imgWidth, imgHeight;
    images = std::vector<stbi_uc *>(numImages);

    for (int i = 0; i < numImages; i++)
    {
        images[i] = stbi_load(filepaths[i].c_str(), &imgWidth, &imgHeight, &channels, STBI_rgb_alpha);

        if (!images[i])
        {
            spdlog::error("stbiload: failed to load texture file {0}", filepaths[i]);
            std::exit(EXIT_FAILURE);
        }

        if (i > 0 && (imgWidth != width || imgHeight != height))
        {
            spdlog::error("Texture: cannot load 3d texture with images of different resolutions");
            std::exit(EXIT_FAILURE);
        }

        width = imgWidth;
        height = imgHeight;
    }

    depth = numImages;
    this->filepaths = filepaths;
    mipLevels = numImages > 1 ? 1 : static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
    imageSize = width * height * 4 * depth;
    format = vk::Format::eR8G8B8A8Unorm;
}

void Texture::createImageView(Device *device, const Builder &builder)
{
    vk::ImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.image = _image;
    imageViewCreateInfo.sType = vk::StructureType::eImageViewCreateInfo;
    imageViewCreateInfo.viewType = builder.depth == 1 ? vk::ImageViewType::e2D : vk::ImageViewType::e3D;
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

    if (_filepaths.size() <= 1)
    {
        textureElement->SetAttribute("filepath", _filepaths[0].c_str());
    }
    else
    {
        for (const std::string &filepath : _filepaths)
        {
            tinyxml2::XMLElement *filepathElement = doc.NewElement("layer");
            textureElement->InsertEndChild(filepathElement);
            filepathElement->SetAttribute("filepath", filepath.c_str());
        }
    }

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
    samplerCreateInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
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

void Texture::bindMany(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout,
                       std::vector<Texture *> textures)
{
    std::vector<vk::DescriptorSet> descriptorSets{};

    for (Texture *texture : textures)
    {
        _boundID = texture->_descriptorSetID;
        descriptorSets.push_back(RenderSystem::getInstance()->getSamplerDescriptorSet(texture->_descriptorSetID));
    }

    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 1, descriptorSets.size(),
                                     descriptorSets.data(), 0, nullptr);
}

} // namespace cmx
