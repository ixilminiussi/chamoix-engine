#include "cmx_g_buffer.h"

// cmx
#include "cmx_camera.h"
#include "cmx_debug_util.h"
#include "cmx_device.h"
#include "cmx_frame_info.h"
#include "cmx_light_environment.h"
#include "cmx_render_system.h"
#include "cmx_swap_chain.h"

// lib
#include <cstdlib>
#include <imgui.h>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

// std
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace cmx
{

GBuffer::GBuffer()
{
}

GBuffer::~GBuffer()
{
    if (!_freed)
    {
        spdlog::error("GBuffer: forgot to free before deletion");
    }
}

void GBuffer::updateAspectRatio(Device *device, const vk::Extent2D &resolution)
{
    RenderSystem *renderSystem = RenderSystem::getInstance();
    device->device().destroyFramebuffer(_framebuffer);
    device->device().destroyRenderPass(_renderPass);
    renderSystem->freeSamplerDescriptor(_samplerDescriptorSetIDs[0]);
    device->device().destroyImageView(_colorImageView);
    device->device().destroyImage(_colorImage);
    device->device().freeMemory(_colorImageMemory);
    renderSystem->freeSamplerDescriptor(_samplerDescriptorSetIDs[1]);
    device->device().destroyImageView(_normalImageView);
    device->device().destroyImage(_normalImage);
    device->device().freeMemory(_normalImageMemory);
    renderSystem->freeSamplerDescriptor(_samplerDescriptorSetIDs[2]);
    device->device().destroyImageView(_depthImageView);
    device->device().destroyImage(_depthImage);
    device->device().freeMemory(_depthImageMemory);

    createTextures(resolution, device);
}

void GBuffer::beginRender(FrameInfo *frameInfo, const LightEnvironment *lightEnvironment) const
{
    static std::array<vk::ClearValue, 3> clearValues{};
    clearValues[0].color = {1.0f, 1.0f, 1.0f, 1.0f};
    clearValues[1].color = {1.0f, 1.0f, 1.0f, 1.0f};
    clearValues[2].depthStencil = vk::ClearDepthStencilValue{1.0f, 0};

    vk::RenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.renderPass = _renderPass;
    renderPassBeginInfo.framebuffer = _framebuffer;
    renderPassBeginInfo.renderArea.extent = _resolution;
    renderPassBeginInfo.clearValueCount = 3;
    renderPassBeginInfo.pClearValues = clearValues.data();

    frameInfo->commandBuffer.beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);

    static vk::Viewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)_resolution.width;
    viewport.height = (float)_resolution.height;
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;

    vk::Rect2D scissor{vk::Offset2D{0, 0}, _resolution};
    frameInfo->commandBuffer.setViewport(0, 1, &viewport);
    frameInfo->commandBuffer.setScissor(0, 1, &scissor);
}

void GBuffer::endRender(FrameInfo *frameInfo) const
{
    frameInfo->commandBuffer.endRenderPass();
}

void GBuffer::createTextures(vk::Extent2D resolution, Device *device)
{
    _resolution = resolution;

    SwapChainSupportDetails swapChainSupport = device->getSwapChainSupport();
    vk::SurfaceFormatKHR surfaceFormat = SwapChain::chooseSwapSurfaceFormat(swapChainSupport.formats);

    _colorFormat = surfaceFormat.format;

    createImages(device);
    createImageViews(device);
    createRenderPass(device);
    createFrameBuffer(device);
    createSamplers(device);
}

void GBuffer::createImages(Device *device)
{
    vk::ImageCreateInfo colorImageInfo{};

    colorImageInfo.sType = vk::StructureType::eImageCreateInfo;
    colorImageInfo.imageType = vk::ImageType::e2D;
    colorImageInfo.extent = vk::Extent3D{_resolution.width, _resolution.height, 1u};
    colorImageInfo.mipLevels = 1;
    colorImageInfo.arrayLayers = 1;
    colorImageInfo.samples = vk::SampleCountFlagBits::e1;
    colorImageInfo.format = _colorFormat;
    colorImageInfo.tiling = vk::ImageTiling::eOptimal;
    colorImageInfo.initialLayout = vk::ImageLayout::eUndefined;
    colorImageInfo.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;

    device->createImageWithInfo(colorImageInfo, {vk::MemoryPropertyFlagBits::eDeviceLocal}, _colorImage,
                                _colorImageMemory);

    vk::ImageCreateInfo normalImageInfo{};

    normalImageInfo.sType = vk::StructureType::eImageCreateInfo;
    normalImageInfo.imageType = vk::ImageType::e2D;
    normalImageInfo.extent = vk::Extent3D{_resolution.width, _resolution.height, 1u};
    normalImageInfo.mipLevels = 1;
    normalImageInfo.arrayLayers = 1;
    normalImageInfo.samples = vk::SampleCountFlagBits::e1;
    normalImageInfo.format = vk::Format::eR16G16B16A16Snorm;
    normalImageInfo.tiling = vk::ImageTiling::eOptimal;
    normalImageInfo.initialLayout = vk::ImageLayout::eUndefined;
    normalImageInfo.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;

    device->createImageWithInfo(normalImageInfo, {vk::MemoryPropertyFlagBits::eDeviceLocal}, _normalImage,
                                _normalImageMemory);

    vk::ImageCreateInfo depthImageInfo{};

    depthImageInfo.sType = vk::StructureType::eImageCreateInfo;
    depthImageInfo.imageType = vk::ImageType::e2D;
    depthImageInfo.extent = vk::Extent3D{_resolution.width, _resolution.height, 1u};
    depthImageInfo.mipLevels = 1;
    depthImageInfo.arrayLayers = 1;
    depthImageInfo.samples = vk::SampleCountFlagBits::e1;
    depthImageInfo.format = vk::Format::eD32Sfloat;
    depthImageInfo.tiling = vk::ImageTiling::eOptimal;
    depthImageInfo.initialLayout = vk::ImageLayout::eUndefined;
    depthImageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;

    device->createImageWithInfo(depthImageInfo, {vk::MemoryPropertyFlagBits::eDeviceLocal}, _depthImage,
                                _depthImageMemory);
}

void GBuffer::createImageViews(Device *device)
{
    vk::ImageViewCreateInfo colorViewInfo{};
    colorViewInfo.image = _colorImage; // The color texture image
    colorViewInfo.viewType = vk::ImageViewType::e2D;
    colorViewInfo.format = _colorFormat;
    colorViewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    colorViewInfo.subresourceRange.baseMipLevel = 0;
    colorViewInfo.subresourceRange.levelCount = 1;
    colorViewInfo.subresourceRange.baseArrayLayer = 0;
    colorViewInfo.subresourceRange.layerCount = 1;

    if (device->device().createImageView(&colorViewInfo, nullptr, &_colorImageView) != vk::Result::eSuccess)
    {
        throw std::runtime_error("GBuffer: failed to create color image colorView!");
    }

    vk::ImageViewCreateInfo normalViewInfo{};
    normalViewInfo.image = _normalImage; // The normal texture image
    normalViewInfo.viewType = vk::ImageViewType::e2D;
    normalViewInfo.format = vk::Format::eR16G16B16A16Snorm;
    normalViewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    normalViewInfo.subresourceRange.baseMipLevel = 0;
    normalViewInfo.subresourceRange.levelCount = 1;
    normalViewInfo.subresourceRange.baseArrayLayer = 0;
    normalViewInfo.subresourceRange.layerCount = 1;

    if (device->device().createImageView(&normalViewInfo, nullptr, &_normalImageView) != vk::Result::eSuccess)
    {
        throw std::runtime_error("GBuffer: failed to create depth image depthView!");
    }

    vk::ImageViewCreateInfo depthViewInfo{};
    depthViewInfo.image = _depthImage; // The depth texture image
    depthViewInfo.viewType = vk::ImageViewType::e2D;
    depthViewInfo.format = vk::Format::eD32Sfloat;
    depthViewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
    depthViewInfo.subresourceRange.baseMipLevel = 0;
    depthViewInfo.subresourceRange.levelCount = 1;
    depthViewInfo.subresourceRange.baseArrayLayer = 0;
    depthViewInfo.subresourceRange.layerCount = 1;

    if (device->device().createImageView(&depthViewInfo, nullptr, &_depthImageView) != vk::Result::eSuccess)
    {
        throw std::runtime_error("GBuffer: failed to create depth image depthView!");
    }
}

void GBuffer::createRenderPass(Device *device)
{
    vk::AttachmentDescription attachments[3]; // color, normal, depth

    // Color
    attachments[0].format = _colorFormat;
    attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
    attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
    attachments[0].initialLayout = vk::ImageLayout::eUndefined;
    attachments[0].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    // Normal
    attachments[1].format = vk::Format::eR16G16B16A16Snorm;
    attachments[1].loadOp = vk::AttachmentLoadOp::eClear;
    attachments[1].storeOp = vk::AttachmentStoreOp::eStore;
    attachments[1].initialLayout = vk::ImageLayout::eUndefined;
    attachments[1].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    // Depth
    attachments[2].format = vk::Format::eD32Sfloat;
    attachments[2].samples = vk::SampleCountFlagBits::e1;
    attachments[2].loadOp = vk::AttachmentLoadOp::eClear;
    attachments[2].storeOp = vk::AttachmentStoreOp::eStore;
    attachments[2].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    attachments[2].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachments[2].initialLayout = vk::ImageLayout::eUndefined;
    attachments[2].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    vk::AttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 2;
    depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::AttachmentReference colorAttachmentRefs[2] = {{0, vk::ImageLayout::eColorAttachmentOptimal},
                                                      {1, vk::ImageLayout::eColorAttachmentOptimal}};

    vk::SubpassDescription subpass{};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpass.colorAttachmentCount = 2;
    subpass.pColorAttachments = colorAttachmentRefs;

    vk::RenderPassCreateInfo renderPassInfo{};
    renderPassInfo.attachmentCount = 3;
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if (device->device().createRenderPass(&renderPassInfo, nullptr, &_renderPass) != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to create depth render pass");
    }

    DebugUtil::nameObject(_renderPass, vk::ObjectType::eRenderPass, "Gbuffer RenderPass");
}

void GBuffer::createFrameBuffer(Device *device)
{
    vk::ImageView attachments[3] = {_colorImageView, _normalImageView, _depthImageView};

    vk::FramebufferCreateInfo framebufferInfo{};
    framebufferInfo.renderPass = _renderPass;
    framebufferInfo.attachmentCount = 3;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = _resolution.width;
    framebufferInfo.height = _resolution.height;
    framebufferInfo.layers = 1;

    if (device->device().createFramebuffer(&framebufferInfo, nullptr, &_framebuffer) != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to create frame buffer");
    }
}

void GBuffer::createSamplers(Device *device)
{
    RenderSystem *renderSystem = RenderSystem::getInstance();

    if (!_colorSampler)
    {
        vk::SamplerCreateInfo colorSamplerCreateInfo{};
        colorSamplerCreateInfo.magFilter = vk::Filter::eLinear;
        colorSamplerCreateInfo.minFilter = vk::Filter::eLinear;
        colorSamplerCreateInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
        colorSamplerCreateInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
        colorSamplerCreateInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
        colorSamplerCreateInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
        colorSamplerCreateInfo.unnormalizedCoordinates = false;
        colorSamplerCreateInfo.compareEnable = true;

        _colorSampler = device->device().createSampler(colorSamplerCreateInfo);
    }

    _samplerDescriptorSetIDs[0] = renderSystem->createSamplerDescriptor(_colorImageView, _colorSampler);

    if (!_normalSampler)
    {
        vk::SamplerCreateInfo normalSamplerCreateInfo{};
        normalSamplerCreateInfo.magFilter = vk::Filter::eLinear;
        normalSamplerCreateInfo.minFilter = vk::Filter::eLinear;
        normalSamplerCreateInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
        normalSamplerCreateInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
        normalSamplerCreateInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
        normalSamplerCreateInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
        normalSamplerCreateInfo.unnormalizedCoordinates = false;
        normalSamplerCreateInfo.compareEnable = true;

        _normalSampler = device->device().createSampler(normalSamplerCreateInfo);
    }

    _samplerDescriptorSetIDs[1] = renderSystem->createSamplerDescriptor(_normalImageView, _normalSampler);

    if (!_depthSampler)
    {
        vk::SamplerCreateInfo depthSamplerCreateInfo{};
        depthSamplerCreateInfo.magFilter = vk::Filter::eLinear;
        depthSamplerCreateInfo.minFilter = vk::Filter::eLinear;
        depthSamplerCreateInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
        depthSamplerCreateInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
        depthSamplerCreateInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
        depthSamplerCreateInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
        depthSamplerCreateInfo.unnormalizedCoordinates = false;
        depthSamplerCreateInfo.compareEnable = true;

        _depthSampler = device->device().createSampler(depthSamplerCreateInfo);
    }

    _samplerDescriptorSetIDs[2] = renderSystem->createSamplerDescriptor(_depthImageView, _depthSampler);
}

void GBuffer::free(Device *device)
{
    _freed = true;

    RenderSystem *renderSystem = RenderSystem::getInstance();

    device->device().destroyFramebuffer(_framebuffer);
    device->device().destroyRenderPass(_renderPass);
    renderSystem->freeSamplerDescriptor(_samplerDescriptorSetIDs[0]);
    device->device().destroySampler(_colorSampler);
    device->device().destroyImageView(_colorImageView);
    device->device().destroyImage(_colorImage);
    device->device().freeMemory(_colorImageMemory);
    renderSystem->freeSamplerDescriptor(_samplerDescriptorSetIDs[1]);
    device->device().destroySampler(_normalSampler);
    device->device().destroyImageView(_normalImageView);
    device->device().destroyImage(_normalImage);
    device->device().freeMemory(_normalImageMemory);
    renderSystem->freeSamplerDescriptor(_samplerDescriptorSetIDs[2]);
    device->device().destroySampler(_depthSampler);
    device->device().destroyImageView(_depthImageView);
    device->device().destroyImage(_depthImage);
    device->device().freeMemory(_depthImageMemory);
}

} // namespace cmx
