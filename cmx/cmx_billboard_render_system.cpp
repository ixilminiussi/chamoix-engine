#include "cmx_billboard_render_system.h"

// cmx
#include "cmx_buffer.h"
#include "cmx_camera.h"
#include "cmx_component.h"
#include "cmx_descriptors.h"
#include "cmx_device.h"
#include "cmx_frame_info.h"
#include "cmx_graphics_manager.h"
#include "cmx_pipeline.h"
#include "cmx_render_system.h"
#include "cmx_renderer.h"
#include "cmx_window.h"

// lib
#include "imgui.h"
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
#include <bits/stdc++.h>
#include <memory>
#include <stdexcept>

namespace cmx
{

void BillboardRenderSystem::initialize()
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

    _dummyBuffer =
        std::make_unique<CmxBuffer>(*_cmxDevice, sizeof(float), 1, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    spdlog::info("BillboardRenderSystem: Successfully initialized!");
}

void BillboardRenderSystem::render(const FrameInfo *frameInfo, std::vector<std::shared_ptr<Component>> &renderQueue,
                                   class GraphicsManager *graphicsManager)
{
    _cmxPipeline->bind(frameInfo->commandBuffer);

    vkCmdBindDescriptorSets(frameInfo->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1,
                            &frameInfo->globalDescriptorSet, 0, nullptr);

    _activeSystem = BILLBOARD_RENDER_SYSTEM;

    VkDeviceSize offsets[] = {0};
    VkBuffer buffer = _dummyBuffer->getBuffer();
    vkCmdBindVertexBuffers(frameInfo->commandBuffer, 0, 1, &buffer, offsets);

    glm::vec3 cameraPosition = frameInfo->camera.getPosition();

    // sort billboards by closest to furthest for transparency
    std::sort(renderQueue.begin(), renderQueue.end(),
              [cameraPosition](const std::shared_ptr<Component> a, const std::shared_ptr<Component> b) {
                  if (a->getRenderZ() == b->getRenderZ())
                  {
                      glm::vec3 differenceA = cameraPosition - a->getAbsoluteTransform().position;
                      float distanceSquaredA = glm::dot(differenceA, differenceA);

                      glm::vec3 differenceB = cameraPosition - b->getAbsoluteTransform().position;
                      float distanceSquaredB = glm::dot(differenceB, differenceB);

                      return distanceSquaredA > distanceSquaredB;
                  }
                  return a->getRenderZ() < b->getRenderZ();
              });

    auto it = renderQueue.begin();
    while (it != renderQueue.end())
    {
        auto renderComponent = *it;

        if (renderComponent->getRequestedRenderSystem() != BILLBOARD_RENDER_SYSTEM)
        {
            it = renderQueue.erase(it);
            graphicsManager->addToQueue(renderComponent);
            continue;
        }
        if (renderComponent->getVisible())
        {
            renderComponent->render(*frameInfo, _pipelineLayout);
        }
        it++;
    }
}

void BillboardRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
{
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(BillboardPushConstant);

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

void BillboardRenderSystem::createPipeline(VkRenderPass renderPass)
{
    assert(_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    CmxPipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.bindingDescriptions.clear();
    pipelineConfig.attributeDescriptions.clear();
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = _pipelineLayout;

    pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
    pipelineConfig.colorBlendAttachment.blendEnable = VK_TRUE;
    pipelineConfig.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    pipelineConfig.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

    _cmxPipeline = std::make_unique<CmxPipeline>(*_cmxDevice.get(), "shaders/billboard.vert.spv",
                                                 "shaders/billboard.frag.spv", pipelineConfig);
}

} // namespace cmx
