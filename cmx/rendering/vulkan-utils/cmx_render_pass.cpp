#include "cmx_render_pass.h"

// cmx
#include "cmx_render_system.h"

// lib
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

// std

namespace cmx
{

RenderPass::RenderPass(Device *device, const vk::Extent2D &resolution,
                       const std::vector<AttachmentInfo> &attachmentInfos, const std::vector<SubpassInfo> &subpassInfos)
    : _resolution{resolution}, _attachmentInfos{attachmentInfos}, _subpassInfos{subpassInfos}
{
    buildRenderTargets(device, _resolution, _attachmentInfos, _subpassInfos);
}

void RenderPass::buildRenderTargets(class Device *device, const vk::Extent2D &resolution,
                                    const std::vector<AttachmentInfo> &attachmentInfos,
                                    const std::vector<SubpassInfo> &subpassInfos)
{
    std::vector<vk::ImageView> imageViews;
    std::vector<vk::AttachmentDescription> attachmentDescriptions;

    static int i = 0;
    for (const AttachmentInfo &attachmentInfo : attachmentInfos)
    {
        RenderTarget renderTarget{.clearValue = attachmentInfo.clearValue, .format = attachmentInfo.format};

        vk::Image image;
        vk::DeviceMemory deviceMemory;
        vk::ImageCreateInfo imageInfo{};

        imageInfo.sType = vk::StructureType::eImageCreateInfo;
        imageInfo.imageType = vk::ImageType::e2D;
        imageInfo.extent = vk::Extent3D{resolution.width, resolution.height, 1u};
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.samples = vk::SampleCountFlagBits::e1;
        imageInfo.format = attachmentInfo.format;
        imageInfo.tiling = vk::ImageTiling::eOptimal;
        imageInfo.initialLayout = vk::ImageLayout::eUndefined;
        imageInfo.usage = attachmentInfo.usage;

        if (imageInfo.format == vk::Format::eR4G4UnormPack8)
        {
            spdlog::critical("yes, {0}", i);
        }
        i++;

        device->createImageWithInfo(imageInfo, {vk::MemoryPropertyFlagBits::eDeviceLocal}, renderTarget.image,
                                    renderTarget.deviceMemory);

        vk::ImageViewCreateInfo viewInfo{};
        viewInfo.image = renderTarget.image;
        viewInfo.viewType = vk::ImageViewType::e2D;
        viewInfo.format = renderTarget.format;
        viewInfo.subresourceRange.aspectMask = attachmentInfo.aspect;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (device->device().createImageView(&viewInfo, nullptr, &renderTarget.imageView) != vk::Result::eSuccess)
        {
            throw std::runtime_error("RenderPass: Failed to create imageView");
        }
        imageViews.push_back(renderTarget.imageView);

        vk::SamplerCreateInfo samplerCreateInfo{};
        samplerCreateInfo.magFilter = vk::Filter::eLinear;
        samplerCreateInfo.minFilter = vk::Filter::eLinear;
        samplerCreateInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
        samplerCreateInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
        samplerCreateInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
        samplerCreateInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
        samplerCreateInfo.unnormalizedCoordinates = false;
        samplerCreateInfo.compareEnable = attachmentInfo.aspect == vk::ImageAspectFlagBits::eDepth ? true : false;

        renderTarget.sampler = device->device().createSampler(samplerCreateInfo);

        renderTarget.descriptorSetID =
            RenderSystem::getInstance()->createSamplerDescriptor(renderTarget.imageView, renderTarget.sampler);

        vk::AttachmentDescription attachmentDescription{};
        attachmentDescription.format = attachmentInfo.format;
        attachmentDescription.samples = vk::SampleCountFlagBits::e1;
        attachmentDescription.loadOp = attachmentInfo.loadOp;
        attachmentDescription.storeOp = attachmentInfo.storeOp;
        attachmentDescription.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        attachmentDescription.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        attachmentDescription.initialLayout = attachmentInfo.initial;
        attachmentDescription.finalLayout = attachmentInfo.final;

        attachmentDescriptions.push_back(attachmentDescription);

        _renderTargets.emplace_back(renderTarget);
    }

    std::vector<std::vector<vk::AttachmentReference>> colorReferences;
    std::vector<std::optional<vk::AttachmentReference>> depthReferences;
    colorReferences.reserve(subpassInfos.size());
    depthReferences.reserve(subpassInfos.size());

    std::vector<vk::SubpassDescription> subpassDescriptions{};
    subpassDescriptions.reserve(subpassInfos.size());

    for (const SubpassInfo &subpassInfo : subpassInfos)
    {
        colorReferences.emplace_back();
        std::vector<vk::AttachmentReference> &colorAttachmentReferences = colorReferences.back();
        for (uint32_t attachmentIndex : subpassInfo.colorAttachmentIndices)
        {
            colorAttachmentReferences.emplace_back(attachmentIndex, vk::ImageLayout::eColorAttachmentOptimal);
        }

        depthReferences.emplace_back();
        std::optional<vk::AttachmentReference> &depthAttachmentReference = depthReferences.back();

        if (subpassInfo.depthAttachmentIndex.has_value())
        {
            depthAttachmentReference = vk::AttachmentReference{subpassInfo.depthAttachmentIndex.value(),
                                                               vk::ImageLayout::eDepthStencilAttachmentOptimal};
        }

        vk::SubpassDescription subpassDescription{};
        subpassDescription.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpassDescription.pDepthStencilAttachment = depthAttachmentReference ? &(*depthAttachmentReference) : nullptr;
        subpassDescription.colorAttachmentCount = colorAttachmentReferences.size();
        subpassDescription.pColorAttachments = colorAttachmentReferences.data();

        subpassDescriptions.push_back(subpassDescription);
    }

    vk::RenderPassCreateInfo renderPassInfo{};
    renderPassInfo.attachmentCount = attachmentDescriptions.size();
    renderPassInfo.pAttachments = attachmentDescriptions.data();
    renderPassInfo.subpassCount = subpassDescriptions.size();
    renderPassInfo.pSubpasses = subpassDescriptions.data();

    if (device->device().createRenderPass(&renderPassInfo, nullptr, &_renderPass) != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to create depth render pass");
    }
    // DebugUtil::nameObject(_renderPass, vk::ObjectType::eRenderPass, "Gbuffer RenderPass");

    std::vector<vk::ImageView> attachments{};

    vk::FramebufferCreateInfo framebufferInfo{};
    framebufferInfo.renderPass = _renderPass;
    framebufferInfo.attachmentCount = imageViews.size();
    framebufferInfo.pAttachments = imageViews.data();
    framebufferInfo.width = resolution.width;
    framebufferInfo.height = resolution.height;
    framebufferInfo.layers = 1;

    if (device->device().createFramebuffer(&framebufferInfo, nullptr, &_framebuffer) != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to create frame buffer");
    }

    _resolution = resolution;
}

RenderPass::~RenderPass()
{
    if (!_freed)
    {
        spdlog::error("RenderPass: forgot to free before deletion");
    }
}

void RenderPass::updateAspectRatio(Device *device, const vk::Extent2D &resolution)
{
    if (resolution == _resolution)
    {
        return;
    }

    _resolution = resolution;

    device->device().destroyFramebuffer(_framebuffer);
    device->device().destroyRenderPass(_renderPass);

    for (RenderTarget &renderTarget : _renderTargets)
    {
        renderTarget.free(device);
    }
    _renderTargets.clear();

    buildRenderTargets(device, resolution, _attachmentInfos, _subpassInfos);
}

void RenderPass::beginRender(vk::CommandBuffer commandBuffer) const
{
    std::vector<vk::ClearValue> clearValues{};

    for (const RenderTarget &renderTarget : _renderTargets)
    {
        clearValues.push_back(renderTarget.clearValue);
    }

    vk::RenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.renderPass = _renderPass;
    renderPassBeginInfo.framebuffer = _framebuffer;
    renderPassBeginInfo.renderArea.extent = _resolution;
    renderPassBeginInfo.clearValueCount = clearValues.size();
    renderPassBeginInfo.pClearValues = clearValues.data();

    commandBuffer.beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);

    static vk::Viewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)_resolution.width;
    viewport.height = (float)_resolution.height;
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;

    vk::Rect2D scissor{vk::Offset2D{0, 0}, _resolution};
    commandBuffer.setViewport(0, 1, &viewport);
    commandBuffer.setScissor(0, 1, &scissor);
}

void RenderPass::endRender(vk::CommandBuffer commandBuffer) const
{
    commandBuffer.endRenderPass();
}

void RenderPass::free(Device *device)
{
    _freed = true;

    device->device().destroyFramebuffer(_framebuffer);
    device->device().destroyRenderPass(_renderPass);

    for (RenderTarget &renderTarget : _renderTargets)
    {
        renderTarget.free(device);
    }
}

void RenderTarget::free(class Device *device)
{
    RenderSystem::getInstance()->freeSamplerDescriptor(descriptorSetID);
    device->device().destroySampler(sampler);
    device->device().destroyImageView(imageView);
    device->device().destroyImage(image);
    device->device().freeMemory(deviceMemory);
}

} // namespace cmx
