#include "cmx_render_component.h"

#include "cmx_actor.h"
#include "cmx_default_render_system.h"
#include "cmx_frame_info.h"
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
    renderZ = 1;
}

RenderComponent::RenderComponent()
{
    renderZ = 1;
}

void RenderComponent::render(FrameInfo &frameInfo, VkPipelineLayout pipelineLayout)
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
    Transform transform = getParent()->getAbsoluteTransform();

    push.modelMatrix = transform.mat4();
    push.normalMatrix = transform.normalMatrix();

    vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData),
                       &push);

    cmxModel->bind(frameInfo.commandBuffer);
    cmxModel->draw(frameInfo.commandBuffer);
}

void RenderComponent::setModel(std::shared_ptr<class CmxModel> newModel)
{
    cmxModel = newModel;
}

} // namespace cmx
