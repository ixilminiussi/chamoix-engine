#include "cmx_default_render_system.h"

#include "cmx_camera_component.h"
#include "cmx_component.h"
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

struct SimplePushConstantData
{
    glm::mat4 transform{1.f};
    alignas(16) glm::vec3 color;
};

CmxDefaultRenderSystem::CmxDefaultRenderSystem(CmxDevice &device, VkRenderPass renderPass) : cmxDevice{device}
{
    createPipelineLayout();
    createPipeline(renderPass);
}

CmxDefaultRenderSystem::~CmxDefaultRenderSystem()
{
    vkDestroyPipelineLayout(cmxDevice.device(), pipelineLayout, nullptr);
}

void CmxDefaultRenderSystem::createPipelineLayout()
{
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    if (vkCreatePipelineLayout(cmxDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void CmxDefaultRenderSystem::createPipeline(VkRenderPass renderPass)
{
    assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    CmxPipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = pipelineLayout;
    cmxPipeline =
        std::make_unique<CmxPipeline>(cmxDevice, "shaders/shader.vert.spv", "shaders/shader.frag.spv", pipelineConfig);
}

void CmxDefaultRenderSystem::render(VkCommandBuffer commandBuffer, std::vector<std::weak_ptr<Component>> &components,
                                    const CameraComponent &camera)
{
    cmxPipeline->bind(commandBuffer);

    auto j = components.begin();

    while (j < components.end())
    {
        if (j->expired())
        {
            j = components.erase(j);
            continue;
        }

        std::shared_ptr<Component> component = j->lock();
        if (component)
        {
            component->render(commandBuffer, pipelineLayout, camera);
        }

        j++;
    }
}

} // namespace cmx
