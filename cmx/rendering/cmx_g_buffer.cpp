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
    device->device().destroyImageView(_albedoImageView);
    device->device().destroyImage(_albedoImage);
    device->device().freeMemory(_albedoImageMemory);
    renderSystem->freeSamplerDescriptor(_samplerDescriptorSetIDs[1]);
    device->device().destroyImageView(_normalImageView);
    device->device().destroyImage(_normalImage);
    device->device().freeMemory(_normalImageMemory);
    renderSystem->freeSamplerDescriptor(_samplerDescriptorSetIDs[2]);
    device->device().destroyImageView(_shadowImageView);
    device->device().destroyImage(_shadowImage);
    device->device().freeMemory(_shadowImageMemory);
    renderSystem->freeSamplerDescriptor(_samplerDescriptorSetIDs[3]);
    device->device().destroyImageView(_depthImageView);
    device->device().destroyImage(_depthImage);
    device->device().freeMemory(_depthImageMemory);

    createTextures(resolution, device);
}

void GBuffer::beginRender(FrameInfo *frameInfo, const LightEnvironment *lightEnvironment) const
{
    static std::array<vk::ClearValue, 4> clearValues{};
    clearValues[0].color = {1.0f, 1.0f, 1.0f, 1.0f};
    clearValues[1].color = {1.0f, 1.0f, 1.0f, 1.0f};
    clearValues[2].color = {1.0f, 1.0f, 1.0f, 1.0f};
    clearValues[3].depthStencil = vk::ClearDepthStencilValue{1.0f, 0};

    vk::RenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.renderPass = _renderPass;
    renderPassBeginInfo.framebuffer = _framebuffer;
    renderPassBeginInfo.renderArea.extent = _resolution;
    renderPassBeginInfo.clearValueCount = 4;
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

    _albedoFormat = surfaceFormat.format;

    createImages(device);
    createImageViews(device);
    createRenderPass(device);
    createFrameBuffer(device);
    createSamplers(device);
}

void GBuffer::createImages(Device *device)
{
    vk::ImageCreateInfo albedoImageInfo{};

    albedoImageInfo.sType = vk::StructureType::eImageCreateInfo;
    albedoImageInfo.imageType = vk::ImageType::e2D;
    albedoImageInfo.extent = vk::Extent3D{_resolution.width, _resolution.height, 1u};
    albedoImageInfo.mipLevels = 1;
    albedoImageInfo.arrayLayers = 1;
    albedoImageInfo.samples = vk::SampleCountFlagBits::e1;
    albedoImageInfo.format = _albedoFormat;
    albedoImageInfo.tiling = vk::ImageTiling::eOptimal;
    albedoImageInfo.initialLayout = vk::ImageLayout::eUndefined;
    albedoImageInfo.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;

    device->createImageWithInfo(albedoImageInfo, {vk::MemoryPropertyFlagBits::eDeviceLocal}, _albedoImage,
                                _albedoImageMemory);

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

    vk::ImageCreateInfo shadowImageInfo{};

    shadowImageInfo.sType = vk::StructureType::eImageCreateInfo;
    shadowImageInfo.imageType = vk::ImageType::e2D;
    shadowImageInfo.extent = vk::Extent3D{_resolution.width, _resolution.height, 1u};
    shadowImageInfo.mipLevels = 1;
    shadowImageInfo.arrayLayers = 1;
    shadowImageInfo.samples = vk::SampleCountFlagBits::e1;
    shadowImageInfo.format = vk::Format::eR16G16B16A16Snorm;
    shadowImageInfo.tiling = vk::ImageTiling::eOptimal;
    shadowImageInfo.initialLayout = vk::ImageLayout::eUndefined;
    shadowImageInfo.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;

    device->createImageWithInfo(shadowImageInfo, {vk::MemoryPropertyFlagBits::eDeviceLocal}, _shadowImage,
                                _shadowImageMemory);

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
    vk::ImageViewCreateInfo albedoViewInfo{};
    albedoViewInfo.image = _albedoImage; // The albedo texture image
    albedoViewInfo.viewType = vk::ImageViewType::e2D;
    albedoViewInfo.format = _albedoFormat;
    albedoViewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    albedoViewInfo.subresourceRange.baseMipLevel = 0;
    albedoViewInfo.subresourceRange.levelCount = 1;
    albedoViewInfo.subresourceRange.baseArrayLayer = 0;
    albedoViewInfo.subresourceRange.layerCount = 1;

    if (device->device().createImageView(&albedoViewInfo, nullptr, &_albedoImageView) != vk::Result::eSuccess)
    {
        throw std::runtime_error("GBuffer: failed to create albedo image albedoView!");
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
        throw std::runtime_error("GBuffer: failed to create normal image depthView!");
    }

    vk::ImageViewCreateInfo shadowViewInfo{};
    shadowViewInfo.image = _shadowImage; // The shadow texture image
    shadowViewInfo.viewType = vk::ImageViewType::e2D;
    shadowViewInfo.format = vk::Format::eR16G16B16A16Snorm;
    shadowViewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    shadowViewInfo.subresourceRange.baseMipLevel = 0;
    shadowViewInfo.subresourceRange.levelCount = 1;
    shadowViewInfo.subresourceRange.baseArrayLayer = 0;
    shadowViewInfo.subresourceRange.layerCount = 1;

    if (device->device().createImageView(&shadowViewInfo, nullptr, &_shadowImageView) != vk::Result::eSuccess)
    {
        throw std::runtime_error("GBuffer: failed to create shadow image depthView!");
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
    vk::AttachmentDescription attachments[4];

    // Color
    attachments[0].format = _albedoFormat;
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
    // Shadow
    attachments[2].format = vk::Format::eR16G16B16A16Snorm;
    attachments[2].loadOp = vk::AttachmentLoadOp::eClear;
    attachments[2].storeOp = vk::AttachmentStoreOp::eStore;
    attachments[2].initialLayout = vk::ImageLayout::eUndefined;
    attachments[2].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    // Depth
    attachments[3].format = vk::Format::eD32Sfloat;
    attachments[3].samples = vk::SampleCountFlagBits::e1;
    attachments[3].loadOp = vk::AttachmentLoadOp::eClear;
    attachments[3].storeOp = vk::AttachmentStoreOp::eStore;
    attachments[3].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    attachments[3].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachments[3].initialLayout = vk::ImageLayout::eUndefined;
    attachments[3].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    vk::AttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 3;
    depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::AttachmentReference albedoAttachmentRefs[3] = {{0, vk::ImageLayout::eColorAttachmentOptimal},
                                                       {1, vk::ImageLayout::eColorAttachmentOptimal},
                                                       {2, vk::ImageLayout::eColorAttachmentOptimal}};

    vk::SubpassDescription subpass{};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpass.colorAttachmentCount = 3;
    subpass.pColorAttachments = albedoAttachmentRefs;

    vk::RenderPassCreateInfo renderPassInfo{};
    renderPassInfo.attachmentCount = 4;
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
    vk::ImageView attachments[4] = {_albedoImageView, _normalImageView, _shadowImageView, _depthImageView};

    vk::FramebufferCreateInfo framebufferInfo{};
    framebufferInfo.renderPass = _renderPass;
    framebufferInfo.attachmentCount = 4;
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

    if (!_albedoSampler)
    {
        vk::SamplerCreateInfo albedoSamplerCreateInfo{};
        albedoSamplerCreateInfo.magFilter = vk::Filter::eLinear;
        albedoSamplerCreateInfo.minFilter = vk::Filter::eLinear;
        albedoSamplerCreateInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
        albedoSamplerCreateInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
        albedoSamplerCreateInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
        albedoSamplerCreateInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
        albedoSamplerCreateInfo.unnormalizedCoordinates = false;
        albedoSamplerCreateInfo.compareEnable = true;

        _albedoSampler = device->device().createSampler(albedoSamplerCreateInfo);
    }

    _samplerDescriptorSetIDs[0] = renderSystem->createSamplerDescriptor(_albedoImageView, _albedoSampler);

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

    if (!_shadowSampler)
    {
        vk::SamplerCreateInfo shadowSamplerCreateInfo{};
        shadowSamplerCreateInfo.magFilter = vk::Filter::eLinear;
        shadowSamplerCreateInfo.minFilter = vk::Filter::eLinear;
        shadowSamplerCreateInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
        shadowSamplerCreateInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
        shadowSamplerCreateInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
        shadowSamplerCreateInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
        shadowSamplerCreateInfo.unnormalizedCoordinates = false;
        shadowSamplerCreateInfo.compareEnable = true;

        _shadowSampler = device->device().createSampler(shadowSamplerCreateInfo);
    }

    _samplerDescriptorSetIDs[2] = renderSystem->createSamplerDescriptor(_shadowImageView, _shadowSampler);

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

    _samplerDescriptorSetIDs[3] = renderSystem->createSamplerDescriptor(_depthImageView, _depthSampler);
}

void GBuffer::free(Device *device)
{
    _freed = true;

    RenderSystem *renderSystem = RenderSystem::getInstance();

    device->device().destroyFramebuffer(_framebuffer);
    device->device().destroyRenderPass(_renderPass);
    renderSystem->freeSamplerDescriptor(_samplerDescriptorSetIDs[0]);
    device->device().destroySampler(_albedoSampler);
    device->device().destroyImageView(_albedoImageView);
    device->device().destroyImage(_albedoImage);
    device->device().freeMemory(_albedoImageMemory);
    renderSystem->freeSamplerDescriptor(_samplerDescriptorSetIDs[1]);
    device->device().destroySampler(_normalSampler);
    device->device().destroyImageView(_normalImageView);
    device->device().destroyImage(_normalImage);
    device->device().freeMemory(_normalImageMemory);
    renderSystem->freeSamplerDescriptor(_samplerDescriptorSetIDs[2]);
    device->device().destroySampler(_shadowSampler);
    device->device().destroyImageView(_shadowImageView);
    device->device().destroyImage(_shadowImage);
    device->device().freeMemory(_shadowImageMemory);
    renderSystem->freeSamplerDescriptor(_samplerDescriptorSetIDs[3]);
    device->device().destroySampler(_depthSampler);
    device->device().destroyImageView(_depthImageView);
    device->device().destroyImage(_depthImage);
    device->device().freeMemory(_depthImageMemory);
}

} // namespace cmx
