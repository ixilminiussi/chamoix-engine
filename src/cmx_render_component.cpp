#include "cmx_render_component.h"

#include "cmx_actor.h"
#include "cmx_model.h"

// lib
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <memory>

namespace cmx
{

RenderComponent::RenderComponent(std::shared_ptr<CmxModel> cmxModel) : cmxModel(cmxModel)
{
}

struct SimplePushConstantData
{
    glm::mat2 transform{1.f};
    glm::vec2 offset;
    alignas(16) glm::vec3 color;
};

void RenderComponent::render(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
{
    if (!getParent())
    {
        spdlog::error("RenderComponent parent is null");
        return;
    };

    if (!getParent()->getVisible())
    {
        return;
    }

    if (!cmxModel)
    {
        spdlog::error("RenderComponent is missing model");
    }

    SimplePushConstantData push{};

    push.transform = getParent()->transform.getMatrix();
    push.offset = getParent()->transform.position;
    push.color = glm::vec3{1.0f, 0.0f, 1.0f};

    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                       sizeof(SimplePushConstantData), &push);

    cmxModel->bind(commandBuffer);
    cmxModel->draw(commandBuffer);
}

void RenderComponent::setModel(std::shared_ptr<class CmxModel> newModel)
{
    cmxModel = newModel;
}

} // namespace cmx
