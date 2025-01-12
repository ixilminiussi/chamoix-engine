#include "cmx_shaded_render_system.h"

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

void ShadedRenderSystem::initialize()
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

    spdlog::info("ShadedRenderSystem: Successfully initialized!");
}

void ShadedRenderSystem::render(const FrameInfo *frameInfo, std::vector<std::shared_ptr<Component>> &renderQueue,
                                class GraphicsManager *graphicsManager)
{
    _cmxPipeline->bind(frameInfo->commandBuffer);

    vkCmdBindDescriptorSets(frameInfo->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1,
                            &frameInfo->globalDescriptorSet, 0, nullptr);

    glm::vec3 cameraPosition = frameInfo->camera.getPosition();

    std::sort(renderQueue.begin(), renderQueue.end(),
              [cameraPosition](const std::shared_ptr<Component> a, const std::shared_ptr<Component> b) {
                  if (a->getRenderZ() == b->getRenderZ())
                  {
                      glm::vec3 differenceA = cameraPosition - a->getAbsoluteTransform().position;
                      float distanceSquaredA = glm::dot(differenceA, differenceA);

                      glm::vec3 differenceB = cameraPosition - b->getAbsoluteTransform().position;
                      float distanceSquaredB = glm::dot(differenceB, differenceB);

                      return distanceSquaredA < distanceSquaredB;
                  }
                  return a->getRenderZ() < b->getRenderZ();
              });

    auto copy = renderQueue;

    auto it = copy.begin();
    while (it != copy.end())
    {
        auto renderComponent = *it;

        if (renderComponent->getRequestedRenderSystem() != SHADED_RENDER_SYSTEM)
        {
            it = copy.erase(it);
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

void ShadedRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
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

void ShadedRenderSystem::createPipeline(VkRenderPass renderPass)
{
    assert(_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    CmxPipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = _pipelineLayout;
    _cmxPipeline = std::make_unique<CmxPipeline>(*_cmxDevice.get(), "shaders/shaded.vert.spv",
                                                 "shaders/shaded.frag.spv", pipelineConfig);
}

} // namespace cmx
