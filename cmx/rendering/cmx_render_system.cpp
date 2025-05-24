#include "cmx_render_system.h"

// cmx
#include "cmx_buffer.h"
#include "cmx_camera.h"
#include "cmx_debug_util.h"
#include "cmx_descriptors.h"
#include "cmx_device.h"
#include "cmx_editor.h"
#include "cmx_frame_info.h"
#include "cmx_game.h"
#include "cmx_light_environment.h"
#include "cmx_render_pass.h"
#include "cmx_renderer.h"
#include "cmx_swap_chain.h"

// lib
#include <imgui.h>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

// std
#include <cstdlib>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace cmx
{

RenderSystem *RenderSystem::_instance{nullptr};

RenderSystem *RenderSystem::getInstance()
{
    if (_instance == nullptr)
    {
        _instance = new RenderSystem();

#ifndef NDEBUG
        _instance->createTexture();
#endif
        _instance->createGBuffer();
    }

    return _instance;
}

RenderSystem::RenderSystem()
{
    _commandBuffer = (VkCommandBuffer_T *)(0);

    _window = &Game::getWindow();
#ifndef NDEBUG
    if (Editor::isActive())
    {
        Editor *editor = Editor::getInstance();
        glm::vec2 size = editor->getSceneViewportSize();
        _resolution.width = size.x;
        _resolution.height = size.y;
    }
    else
    {
#endif
        _resolution = _window->getExtent();
#ifndef NDEBUG
    }
#endif
    _device = std::make_unique<Device>(*_window);
    _renderer = std::make_unique<Renderer>(*_window, *_device.get());

    _uboBuffers = std::vector<std::unique_ptr<Buffer>>{SwapChain::MAX_FRAMES_IN_FLIGHT};

    _globalDescriptorSets = std::vector<vk::DescriptorSet>{SwapChain::MAX_FRAMES_IN_FLIGHT};
    _samplerDescriptorPool = DescriptorPool::Builder(*_device.get())
                                 .setMaxSets(MAX_SAMPLER_SETS)
                                 .addPoolSize(vk::DescriptorType::eCombinedImageSampler, 100)
                                 .build();

    _globalSetLayout = DescriptorSetLayout::Builder(*_device.get())
                           .addBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eAllGraphics)
                           .build();

    _samplerDescriptorSetLayout =
        DescriptorSetLayout::Builder(*_device.get())
            .addBinding(0, vk::DescriptorType::eCombinedImageSampler, {vk::ShaderStageFlagBits::eFragment})
            .build();

    _samplerDescriptorSets.reserve(MAX_SAMPLER_SETS);

    initializeUbo();
}

RenderSystem::~RenderSystem()
{
}

void RenderSystem::initializeUbo()
{
    for (size_t i = 0; i < _uboBuffers.size(); i++)
    {
        _uboBuffers[i] =
            std::make_unique<Buffer>(*_device.get(), sizeof(GlobalUbo), 1, vk::BufferUsageFlagBits::eUniformBuffer,
                                     vk::MemoryPropertyFlagBits::eHostVisible);
        _uboBuffers[i]->map();
    }

    _globalPool = DescriptorPool::Builder(*_device.get())
                      .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
                      .addPoolSize(vk::DescriptorType::eUniformBuffer, SwapChain::MAX_FRAMES_IN_FLIGHT)
                      .build();

    _globalSetLayout = DescriptorSetLayout::Builder(*_device.get())
                           .addBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eAllGraphics)
                           .build();

    for (size_t i = 0; i < _globalDescriptorSets.size(); i++)
    {
        auto bufferInfo = _uboBuffers[i]->descriptorInfo();
        DescriptorWriter(*_globalSetLayout, *_globalPool).writeBuffer(0, &bufferInfo).build(_globalDescriptorSets[i]);
    }
}

void RenderSystem::closeWindow()
{
    spdlog::info("global release");

#ifndef NDEBUG
    freeImages();
#endif
    _gBuffer->free(_device.get());
    _globalPool->free();
    _samplerDescriptorPool->free();
    _device->device().destroyDescriptorSetLayout(_samplerDescriptorSetLayout->getDescriptorSetLayout());

    _renderer->free();
    delete _renderer.release();

    for (auto &buffer : _uboBuffers)
    {
        buffer->free();
    }
    _uboBuffers.clear();

    delete _device.release();
    delete _window;
}

size_t RenderSystem::createSamplerDescriptor(vk::ImageView imageView, vk::Sampler sampler)
{
    vk::DescriptorSet descriptorSet;

    vk::DescriptorSetAllocateInfo setAllocInfo{};
    setAllocInfo.descriptorPool = _samplerDescriptorPool->getDescriptorPool();
    setAllocInfo.descriptorSetCount = 1;
    setAllocInfo.pSetLayouts = &(_samplerDescriptorSetLayout->getDescriptorSetLayout());

    if (_device->device().allocateDescriptorSets(&setAllocInfo, &descriptorSet) != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to allocate texture descriptor set.");
    }

    vk::DescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    imageInfo.imageView = imageView;
    imageInfo.sampler = sampler;
    // Write info
    vk::WriteDescriptorSet descriptorWrite{};
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    _device->device().updateDescriptorSets(1, &descriptorWrite, 0, nullptr);

    _samplerDescriptorSets.push_back(descriptorSet);
    return _samplerDescriptorSets.size() - 1;
}

void RenderSystem::freeSamplerDescriptor(size_t descriptorSetID)
{
    vk::DescriptorSet &descriptorSet = getSamplerDescriptorSet(descriptorSetID);
    _device->device().freeDescriptorSets(_samplerDescriptorPool->getDescriptorPool(), 1, &descriptorSet);
}

vk::DescriptorSet &RenderSystem::getSamplerDescriptorSet(size_t index)
{
    if (index >= _samplerDescriptorSets.size())
    {
        throw std::out_of_range("RenderSystem: invalid sampler descriptor set index");
    }

    return _samplerDescriptorSets[index];
}

void RenderSystem::checkAspectRatio(Camera *camera)
{
    vk::Extent2D newResolution{};
#ifndef NDEBUG
    if (Editor::isActive())
    {
        Editor *editor = Editor::getInstance();
        glm::vec2 size = editor->getSceneViewportSize();
        newResolution.width = size.x;
        newResolution.height = size.y;
    }
    else
    {
#endif
        newResolution = _window->getExtent();
#ifndef NDEBUG
    }
#endif

    if (newResolution == _resolution)
    {
        return;
    }

    _resolution = newResolution;

    float aspect = static_cast<float>(_resolution.width) / static_cast<float>(_resolution.height);
    camera->updateAspectRatio(aspect);

    _gBuffer->updateAspectRatio(_device.get(), _resolution);
    _device->device().destroyFramebuffer(_framebuffer);
    _device->device().destroyRenderPass(_renderPass);
    freeSamplerDescriptor(_samplerDescriptorSetID);
    _device->device().destroyImageView(_imageView);
    _device->device().destroyImage(_image);
    _device->device().freeMemory(_imageMemory);

    createTexture();
}

FrameInfo *RenderSystem::beginCommandBuffer()
{
    FrameInfo *frameInfo{nullptr};

    if ((_commandBuffer = _renderer->beginFrame()))
    {
        int frameIndex = _renderer->getFrameIndex();
        frameInfo = new FrameInfo{frameIndex, _commandBuffer, _globalDescriptorSets[frameIndex]};
    }

    return frameInfo;
}

void RenderSystem::beginRender(FrameInfo *frameInfo, const LightEnvironment *lightEnvironment) const
{
    _gBuffer->beginRender(frameInfo->commandBuffer);
}

void RenderSystem::endRender(FrameInfo *frameInfo) const
{
    _gBuffer->endRender(frameInfo->commandBuffer);
}

void RenderSystem::beginPostProcess(FrameInfo *frameInfo) const
{
#ifndef NDEBUG
    if (Editor::isActive())
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
    else
    {
#endif
        _renderer->beginSwapChainRenderPass(frameInfo->commandBuffer);
#ifndef NDEBUG
    }
#endif
}

void RenderSystem::endPostProcess(struct FrameInfo *frameInfo) const
{
#ifndef NDEBUG
    if (Editor::isActive())
    {
        frameInfo->commandBuffer.endRenderPass();

        _renderer->beginSwapChainRenderPass(frameInfo->commandBuffer);

        Editor *editor = Editor::getInstance();
        editor->render(*frameInfo);
    }
#endif

    _renderer->endSwapChainRenderPass(frameInfo->commandBuffer);
    _renderer->endFrame();

    _device->device().waitIdle();
}

vk::RenderPass RenderSystem::getRenderPass()
{
#ifndef NDEBUG
    if (Editor::isActive())
    {
        return _renderPass;
    }
#endif
    return _renderer->getSwapChainRenderPass();
}

void RenderSystem::createGBuffer()
{
#ifndef NDEBUG
    if (Editor::isActive())
    {
        Editor *editor = Editor::getInstance();
        glm::vec2 size = editor->getSceneViewportSize();
        _resolution.width = size.x;
        _resolution.height = size.y;
    }
    else
    {
#endif
        _resolution = _window->getExtent();
#ifndef NDEBUG
    }
#endif

    SwapChainSupportDetails swapChainSupport = _device->getSwapChainSupport();
    vk::SurfaceFormatKHR surfaceFormat = SwapChain::chooseSwapSurfaceFormat(swapChainSupport.formats);
    surfaceFormat = vk::Format::eR16G16B16A16Snorm;

    _gBuffer = std::make_unique<RenderPass>(
        _device.get(), _resolution,
        std::vector<AttachmentInfo>{
            {.format = surfaceFormat.format,
             .usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
             .final = vk::ImageLayout::eShaderReadOnlyOptimal},
            {.format = vk::Format::eR16G16B16A16Snorm,
             .usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
             .final = vk::ImageLayout::eShaderReadOnlyOptimal},
            {.format = vk::Format::eR16G16B16A16Snorm,
             .usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
             .final = vk::ImageLayout::eShaderReadOnlyOptimal},
            {.aspect = vk::ImageAspectFlagBits::eDepth,
             .clearValue = {vk::ClearDepthStencilValue{1.f, 0}},
             .format = vk::Format::eD32Sfloat,
             .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
             .final = vk::ImageLayout::eShaderReadOnlyOptimal}},
        std::vector<SubpassInfo>{{.colorAttachmentIndices = {0, 1, 2}, .depthAttachmentIndex = 3}});
}

void RenderSystem::writeUbo(FrameInfo *frameInfo, GlobalUbo *ubo)
{
    _uboBuffers[frameInfo->frameIndex]->writeToBuffer(ubo);
    _uboBuffers[frameInfo->frameIndex]->flush();
}

Device *RenderSystem::getDevice()
{
    return _device.get();
}

#ifndef NDEBUG
void RenderSystem::createTexture()
{
    if (Editor::isActive())
    {
        Editor *editor = Editor::getInstance();
        glm::vec2 size = editor->getSceneViewportSize();
        _resolution.width = size.x;
        _resolution.height = size.y;
    }

    SwapChainSupportDetails swapChainSupport = _device->getSwapChainSupport();
    vk::SurfaceFormatKHR surfaceFormat = SwapChain::chooseSwapSurfaceFormat(swapChainSupport.formats);

    _format = surfaceFormat.format;

    createImage();
    createImageView();
    createRenderPass();
    createFrameBuffer();
    createSampler();
}

void RenderSystem::createImage()
{
    vk::ImageCreateInfo imageInfo{};

    imageInfo.sType = vk::StructureType::eImageCreateInfo;
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.extent = vk::Extent3D{_resolution.width, _resolution.height, 1u};
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = vk::SampleCountFlagBits::e1;
    imageInfo.format = _format;
    imageInfo.tiling = vk::ImageTiling::eOptimal;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageInfo.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;

    _device->createImageWithInfo(imageInfo, {vk::MemoryPropertyFlagBits::eDeviceLocal}, _image, _imageMemory);
}

void RenderSystem::createImageView()
{
    vk::ImageViewCreateInfo viewInfo{};
    viewInfo.image = _image; // The color texture image
    viewInfo.viewType = vk::ImageViewType::e2D;
    viewInfo.format = _format;
    viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (_device->device().createImageView(&viewInfo, nullptr, &_imageView) != vk::Result::eSuccess)
    {
        throw std::runtime_error("RenderSystem: failed to create imageView!");
    }
}

void RenderSystem::createRenderPass()
{
    vk::AttachmentDescription attachment; // color, normal, depth

    // Color
    attachment.format = _format;
    attachment.loadOp = vk::AttachmentLoadOp::eClear;
    attachment.storeOp = vk::AttachmentStoreOp::eStore;
    attachment.initialLayout = vk::ImageLayout::eUndefined;
    attachment.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    vk::AttachmentReference attachmentRef = {0, vk::ImageLayout::eColorAttachmentOptimal};

    vk::SubpassDescription subpass{};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.pDepthStencilAttachment = nullptr;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attachmentRef;

    vk::RenderPassCreateInfo renderPassInfo{};
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &attachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if (_device->device().createRenderPass(&renderPassInfo, nullptr, &_renderPass) != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to create depth render pass");
    }

    DebugUtil::nameObject(_renderPass, vk::ObjectType::eRenderPass, "Final RenderPass");
}

void RenderSystem::createFrameBuffer()
{
    vk::ImageView attachment = _imageView;

    vk::FramebufferCreateInfo framebufferInfo{};
    framebufferInfo.renderPass = _renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = &attachment;
    framebufferInfo.width = _resolution.width;
    framebufferInfo.height = _resolution.height;
    framebufferInfo.layers = 1;

    if (_device->device().createFramebuffer(&framebufferInfo, nullptr, &_framebuffer) != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to create frame buffer");
    }
}

void RenderSystem::createSampler()
{
    if (!_sampler)
    {
        vk::SamplerCreateInfo samplerCreateInfo{};
        samplerCreateInfo.magFilter = vk::Filter::eLinear;
        samplerCreateInfo.minFilter = vk::Filter::eLinear;
        samplerCreateInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
        samplerCreateInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
        samplerCreateInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
        samplerCreateInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
        samplerCreateInfo.unnormalizedCoordinates = false;
        samplerCreateInfo.compareEnable = true;

        _sampler = _device->device().createSampler(samplerCreateInfo);
    }

    _samplerDescriptorSetID = createSamplerDescriptor(_imageView, _sampler);
}

void RenderSystem::freeImages()
{
    _device->device().destroyFramebuffer(_framebuffer);
    _device->device().destroyRenderPass(_renderPass);
    freeSamplerDescriptor(_samplerDescriptorSetID);
    _device->device().destroySampler(_sampler);
    _device->device().destroyImageView(_imageView);
    _device->device().destroyImage(_image);
    _device->device().freeMemory(_imageMemory);
}
#endif

} // namespace cmx
