#include "cmx_render_component.h"

#include "cmx_actor.h"
#include "cmx_camera_component.h"
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
    renderZ = 0;
}

RenderComponent::RenderComponent()
{
    renderZ = 0;
}

struct SimplePushConstantData
{
    glm::mat4 transform{1.f};
    glm::mat4 normalMatrix{1.f};
};

void RenderComponent::render(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout,
                             const class CameraComponent &camera)
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

    auto projectionView = camera.getProjection() * camera.getView();

    SimplePushConstantData push{};
    Transform transform = getParent()->getAbsoluteTransform();

    push.transform = projectionView * transform.mat4();
    push.normalMatrix = transform.normalMatrix();

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
