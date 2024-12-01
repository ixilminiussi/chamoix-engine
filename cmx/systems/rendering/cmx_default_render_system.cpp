#include "cmx_default_render_system.h"

#include "cmx_component.h"
#include "cmx_frame_info.h"
#include "cmx_pipeline.h"
#include "cmx_world.h"

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

DefaultRenderSystem::DefaultRenderSystem(CmxDevice &device, VkRenderPass renderPass,
                                         VkDescriptorSetLayout globalSetLayout)
    : cmxDevice{device}
{
    createPipelineLayout(globalSetLayout);
    createPipeline(renderPass);
}

DefaultRenderSystem::~DefaultRenderSystem()
{
    vkDestroyPipelineLayout(cmxDevice.device(), pipelineLayout, nullptr);
}

void DefaultRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
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
    if (vkCreatePipelineLayout(cmxDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void DefaultRenderSystem::createPipeline(VkRenderPass renderPass)
{
    assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    CmxPipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = pipelineLayout;
    cmxPipeline =
        std::make_unique<CmxPipeline>(cmxDevice, "shaders/shader.vert.spv", "shaders/shader.frag.spv", pipelineConfig);
}

void DefaultRenderSystem::render(FrameInfo &frameInfo, std::vector<std::weak_ptr<Component>> &renderQueue)
{
    cmxPipeline->bind(frameInfo.commandBuffer);

    vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
                            &frameInfo.globalDescriptorSet, 0, nullptr);

    auto j = renderQueue.begin();

    while (j < renderQueue.end())
    {
        if (j->expired())
        {
            j = renderQueue.erase(j);
            continue;
        }

        std::shared_ptr<Component> component = j->lock();
        if (component)
        {
            component->render(frameInfo, pipelineLayout);
        }

        j++;
    }
}

} // namespace cmx
