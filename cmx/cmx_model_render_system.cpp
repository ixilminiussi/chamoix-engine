#include "cmx_model_render_system.h"

// cmx
#include "cmx/cmx_billboard_render_system.h"
#include "cmx/cmx_render_system.h"
#include "cmx_buffer.h"
#include "cmx_camera.h"
#include "cmx_component.h"
#include "cmx_descriptors.h"
#include "cmx_device.h"
#include "cmx_frame_info.h"
#include "cmx_pipeline.h"
#include "cmx_renderer.h"
#include "cmx_window.h"

// lib
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <glm/ext/scalar_constants.hpp>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <memory>
#include <stdexcept>

namespace cmx
{

void ModelRenderSystem::initialize()
{
    _globalPool = CmxDescriptorPool::Builder(*_cmxDevice.get())
                      .setMaxSets(CmxSwapChain::MAX_FRAMES_IN_FLIGHT)
                      .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, CmxSwapChain::MAX_FRAMES_IN_FLIGHT)
                      .build();

    for (int i = 0; i < _uboBuffers.size(); i++)
    {
        _uboBuffers[i] =
            std::make_unique<CmxBuffer>(*_cmxDevice.get(), sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        _uboBuffers[i]->map();
    }

    auto globalSetLayout = CmxDescriptorSetLayout::Builder(*_cmxDevice.get())
                               .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                               .build();
    for (int i = 0; i < _globalDescriptorSets.size(); i++)
    {
        auto bufferInfo = _uboBuffers[i]->descriptorInfo();
        CmxDescriptorWriter(*globalSetLayout, *_globalPool).writeBuffer(0, &bufferInfo).build(_globalDescriptorSets[i]);
    }

    createPipelineLayout(globalSetLayout->getDescriptorSetLayout());
    createPipeline(_cmxRenderer->getSwapChainRenderPass());

    spdlog::info("ModelRenderSystem: Successfully initialized!");
}

void ModelRenderSystem::render(FrameInfo *frameInfo, std::shared_ptr<Component> renderComponent)
{
    if (_activeSystem != MODEL_RENDER_SYSTEM)
    {
        _cmxPipeline->bind(frameInfo->commandBuffer);

        vkCmdBindDescriptorSets(frameInfo->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1,
                                &frameInfo->globalDescriptorSet, 0, nullptr);

        _activeSystem = MODEL_RENDER_SYSTEM;
    }

    renderComponent->render(*frameInfo, _pipelineLayout);
}

void ModelRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
{
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    if (vkCreatePipelineLayout(_cmxDevice->device(), &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void ModelRenderSystem::createPipeline(VkRenderPass renderPass)
{
    assert(_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    CmxPipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = _pipelineLayout;
    _cmxPipeline = std::make_unique<CmxPipeline>(*_cmxDevice.get(), "shaders/shader.vert.spv",
                                                 "shaders/shader.frag.spv", pipelineConfig);
}

} // namespace cmx
