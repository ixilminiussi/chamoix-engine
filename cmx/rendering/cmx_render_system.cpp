#include "cmx_render_system.h"

// cmx
#include "cmx_buffer.h"
#include "cmx_camera.h"
#include "cmx_descriptors.h"
#include "cmx_device.h"
#include "cmx_frame_info.h"
#include "cmx_game.h"
#include "cmx_light_environment.h"
#include "cmx_pipeline.h"
#include "cmx_renderer.h"
#include "cmx_window.h"

// lib
#include <imgui.h>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

// std
#include <cstdlib>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace cmx
{

RenderSystem *RenderSystem::_instance{nullptr};

RenderSystem *RenderSystem::getInstance()
{
    if (_instance == nullptr)
    {
        _instance = new RenderSystem();
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
    for (int i = 0; i < _uboBuffers.size(); i++)
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

    for (int i = 0; i < _globalDescriptorSets.size(); i++)
    {
        auto bufferInfo = _uboBuffers[i]->descriptorInfo();
        DescriptorWriter(*_globalSetLayout, *_globalPool).writeBuffer(0, &bufferInfo).build(_globalDescriptorSets[i]);
    }
}

void RenderSystem::closeWindow()
{
    spdlog::info("global release");

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

void RenderSystem::checkAspectRatio(class Camera *camera)
{
    float aspect = _renderer->getAspectRatio();
    camera->updateAspectRatio(aspect);
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

void RenderSystem::beginRender(FrameInfo *frameInfo, const LightEnvironment *lightEnvironment)
{
    _renderer->beginSwapChainRenderPass(_commandBuffer);
}

void RenderSystem::writeUbo(class FrameInfo *frameInfo, class GlobalUbo *ubo)
{
    _uboBuffers[frameInfo->frameIndex]->writeToBuffer(ubo);
    _uboBuffers[frameInfo->frameIndex]->flush();
}

void RenderSystem::endRender()
{
    _renderer->endSwapChainRenderPass(_commandBuffer);
    _renderer->endFrame();

    _device->device().waitIdle();
}

Device *RenderSystem::getDevice()
{
    return _device.get();
}

} // namespace cmx
