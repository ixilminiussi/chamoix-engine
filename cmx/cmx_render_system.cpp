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
#include <vulkan/vulkan_core.h>

// std
#include <cstdlib>
#include <vulkan/vulkan_enums.hpp>

namespace cmx
{

uint8_t RenderSystem::_activeSystem = NULL_RENDER_SYSTEM;

vk::CommandBuffer RenderSystem::_commandBuffer = (VkCommandBuffer_T *)(0);

Window *RenderSystem::_window = &Game::getWindow();

std::unique_ptr<Device> RenderSystem::_device = std::make_unique<Device>(*_window);

std::unique_ptr<Renderer> RenderSystem::_renderer = std::make_unique<Renderer>(*_window, *_device.get());

std::vector<std::unique_ptr<Buffer>> RenderSystem::_uboBuffers =
    std::vector<std::unique_ptr<Buffer>>{SwapChain::MAX_FRAMES_IN_FLIGHT};

std::vector<vk::DescriptorSet> RenderSystem::_globalDescriptorSets =
    std::vector<vk::DescriptorSet>{SwapChain::MAX_FRAMES_IN_FLIGHT};

std::unique_ptr<DescriptorPool> RenderSystem::_samplerDescriptorPool =
    DescriptorPool::Builder(*_device.get())
        .setMaxSets(100)
        .addPoolSize(vk::DescriptorType::eCombinedImageSampler, 100)
        .build();

std::unique_ptr<DescriptorSetLayout> RenderSystem::_samplerDescriptorSetLayout =
    DescriptorSetLayout::Builder(*_device.get())
        .addBinding(0, vk::DescriptorType::eCombinedImageSampler, {vk::ShaderStageFlagBits::eFragment})
        .build();

std::vector<vk::DescriptorSet> RenderSystem::_samplerDescriptorSets{};

bool RenderSystem::_uboInitialized = false;

RenderSystem::RenderSystem()
{
}

RenderSystem::~RenderSystem()
{
    if (!_freed)
    {
        spdlog::error("RenderSystem: forgot to free before deletion");
    }
}

void RenderSystem::free()
{
    _pipeline->free();
    _globalPool->free();

    _device->device().destroyPipelineLayout(_pipelineLayout, nullptr);

    _freed = true;
}

void RenderSystem::initializeUbo()
{
    if (_uboInitialized)
        return;

    for (int i = 0; i < _uboBuffers.size(); i++)
    {
        _uboBuffers[i] =
            std::make_unique<Buffer>(*_device.get(), sizeof(GlobalUbo), 1, vk::BufferUsageFlagBits::eUniformBuffer,
                                     vk::MemoryPropertyFlagBits::eHostVisible);
        _uboBuffers[i]->map();
    }

    _uboInitialized = true;
}

void RenderSystem::closeWindow()
{
    spdlog::info("global release");

    _samplerDescriptorPool->free();
    _device->device().destroyDescriptorSetLayout(_samplerDescriptorSetLayout->getDescriptorSetLayout());

    _renderer->free();
    delete _renderer.release();

    // _device->device().freeCommandBuffers(_device->getCommandPool(), 1u, &_commandBuffer);

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
        spdlog::error("RenderSystem: invalid sampler descriptor set index");
        std::exit(EXIT_FAILURE);
    }

    return _samplerDescriptorSets[index];
}

void RenderSystem::editor(int i)
{
    ImGui::PushID(i);
    ImGui::Checkbox("##", &_visible);
    ImGui::PopID();
}

void RenderSystem::checkAspectRatio(class Camera *camera)
{
    float aspect = _renderer->getAspectRatio();
    camera->updateAspectRatio(aspect);
}

FrameInfo *RenderSystem::beginRender(Camera *camera, const LightEnvironment *lightEnvironment)
{
    FrameInfo *frameInfo;

    if ((_commandBuffer = _renderer->beginFrame()))
    {
        int frameIndex = _renderer->getFrameIndex();
        frameInfo = new FrameInfo{frameIndex, _commandBuffer, *camera, _globalDescriptorSets[frameIndex]};
        // update
        GlobalUbo ubo{};
        ubo.projection = camera->getProjection();
        ubo.view = camera->getView();

        if (lightEnvironment != nullptr)
        {
            lightEnvironment->populateUbo(&ubo);
        }

        _uboBuffers[frameIndex]->writeToBuffer(&ubo);
        _uboBuffers[frameIndex]->flush();

        // render
        _renderer->beginSwapChainRenderPass(_commandBuffer);
    }

    return frameInfo;
}

void RenderSystem::endRender()
{
    _activeSystem = NULL_RENDER_SYSTEM;

    _renderer->endSwapChainRenderPass(_commandBuffer);
    _renderer->endFrame();
    vkDeviceWaitIdle(_device->device());
}

Device *RenderSystem::getDevice()
{
    return _device.get();
}

} // namespace cmx
