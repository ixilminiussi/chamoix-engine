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
#include "cmx_material.h"
#include "cmx_post_blur_material.h"
#include "cmx_post_ssao_material.h"
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
        _instance->createViewport();
#endif
        _instance->createGBuffer();
        _instance->createSSAOBuffers();
    }

    return _instance;
}

RenderSystem::RenderSystem()
{
    _commandBuffer = (VkCommandBuffer_T *)(0);

    _window = &Game::getWindow();
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
    _viewport->free(_device.get());
#endif
    _gBuffer->free(_device.get());
    _ssaoMaterials[0]->free();
    _ssaoMaterials[1]->free();
    _ssaoBuffers[0]->free(_device.get());
    _ssaoBuffers[1]->free(_device.get());
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
    vk::Extent2D resolution = getResolution();
#ifndef NDEBUG
    _viewport->updateAspectRatio(_device.get(), resolution);
#endif

    float aspect = static_cast<float>(resolution.width) / static_cast<float>(resolution.height);
    camera->updateAspectRatio(aspect);

    _gBuffer->updateAspectRatio(_device.get(), resolution);
    _ssaoBuffers[0]->updateAspectRatio(_device.get(), resolution);
    _ssaoBuffers[1]->updateAspectRatio(_device.get(), resolution);
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
        vk::RenderPass renderPass = _viewport->getRenderPass();
        vk::Framebuffer framebuffer = _viewport->getFrameBuffer();
        vk::Extent2D resolution = _viewport->getResolution();
        static std::array<vk::ClearValue, 3> clearValues{};
        clearValues[0].color = {1.0f, 1.0f, 1.0f, 1.0f};
        clearValues[1].color = {1.0f, 1.0f, 1.0f, 1.0f};
        clearValues[2].depthStencil = vk::ClearDepthStencilValue{1.0f, 0};

        vk::RenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.renderPass = renderPass;
        renderPassBeginInfo.framebuffer = framebuffer;
        renderPassBeginInfo.renderArea.extent = resolution;
        renderPassBeginInfo.clearValueCount = 3;
        renderPassBeginInfo.pClearValues = clearValues.data();

        frameInfo->commandBuffer.beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);

        static vk::Viewport viewport;
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)resolution.width;
        viewport.height = (float)resolution.height;
        viewport.minDepth = 0.f;
        viewport.maxDepth = 1.f;

        vk::Rect2D scissor{vk::Offset2D{0, 0}, resolution};
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

vk::Extent2D RenderSystem::getResolution() const
{
    vk::Extent2D resolution;
#ifndef NDEBUG
    if (Editor::isActive())
    {
        Editor *editor = Editor::getInstance();
        glm::vec2 size = editor->getSceneViewportSize();
        resolution.width = size.x;
        resolution.height = size.y;
    }
    else
    {
#endif
        resolution = _window->getExtent();
#ifndef NDEBUG
    }
#endif
    return resolution;
}

vk::RenderPass RenderSystem::getRenderPass() const
{
#ifndef NDEBUG
    if (Editor::isActive())
    {
        return _viewport->getRenderPass();
    }
#endif
    return _renderer->getSwapChainRenderPass();
}

#ifndef NDEBUG
void RenderSystem::createViewport()
{
    vk::Extent2D resolution = getResolution();

    SwapChainSupportDetails swapChainSupport = _device->getSwapChainSupport();
    vk::SurfaceFormatKHR surfaceFormat = SwapChain::chooseSwapSurfaceFormat(swapChainSupport.formats);
    surfaceFormat = vk::Format::eR16G16B16A16Snorm;

    _viewport =
        std::make_unique<RenderPass>(_device.get(), resolution,
                                     std::vector<AttachmentInfo>{{.format = surfaceFormat.format,
                                                                  .usage = vk::ImageUsageFlagBits::eColorAttachment |
                                                                           vk::ImageUsageFlagBits::eSampled,
                                                                  .final = vk::ImageLayout::eShaderReadOnlyOptimal}},
                                     std::vector<SubpassInfo>{{.colorAttachmentIndices = {0}}});
}
#endif

void RenderSystem::createSSAOBuffers()
{
    vk::Extent2D resolution = getResolution();

    _ssaoBuffers[0] = new RenderPass(_device.get(), resolution,
                                     std::vector<AttachmentInfo>{{.format = vk::Format::eR16Snorm,
                                                                  .usage = vk::ImageUsageFlagBits::eColorAttachment |
                                                                           vk::ImageUsageFlagBits::eSampled,
                                                                  .final = vk::ImageLayout::eShaderReadOnlyOptimal}},
                                     std::vector<SubpassInfo>{{.colorAttachmentIndices = {0}}});
    _ssaoBuffers[1] = new RenderPass(_device.get(), resolution,
                                     std::vector<AttachmentInfo>{{.format = vk::Format::eR16Snorm,
                                                                  .usage = vk::ImageUsageFlagBits::eColorAttachment |
                                                                           vk::ImageUsageFlagBits::eSampled,
                                                                  .final = vk::ImageLayout::eShaderReadOnlyOptimal}},
                                     std::vector<SubpassInfo>{{.colorAttachmentIndices = {0}}});

    _ssaoMaterials[0] = new PostSSAOMaterial();
    _ssaoMaterials[0]->initialize();
    _ssaoMaterials[1] = new PostBlurMaterial();
    _ssaoMaterials[1]->initialize();
}

void RenderSystem::drawSSAO(FrameInfo *frameInfo) const
{
    _ssaoBuffers[0]->beginRender(frameInfo->commandBuffer);
    _ssaoMaterials[0]->bind(frameInfo, nullptr);
    frameInfo->commandBuffer.draw(6, 1, 0, 0);
    _ssaoBuffers[0]->endRender(frameInfo->commandBuffer);

    // blur pass 1
    _ssaoBuffers[1]->beginRender(frameInfo->commandBuffer);
    _ssaoMaterials[1]->bind(frameInfo, nullptr);
    frameInfo->commandBuffer.draw(6, 1, 0, 0);
    _ssaoBuffers[1]->endRender(frameInfo->commandBuffer);
}

void RenderSystem::createGBuffer()
{
    vk::Extent2D resolution = getResolution();

    SwapChainSupportDetails swapChainSupport = _device->getSwapChainSupport();
    vk::SurfaceFormatKHR surfaceFormat = SwapChain::chooseSwapSurfaceFormat(swapChainSupport.formats);
    surfaceFormat = vk::Format::eR16G16B16A16Snorm;

    _gBuffer = std::make_unique<RenderPass>(
        _device.get(), resolution,
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

} // namespace cmx
