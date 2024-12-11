#include "cmx_mesh_component.h"

#include "cmx_actor.h"
#include "cmx_default_render_system.h"
#include "cmx_frame_info.h"
#include "cmx_model.h"
#include "tinyxml2.h"

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

MeshComponent::MeshComponent()
{
    renderZ = 1;
}

MeshComponent::MeshComponent(std::shared_ptr<CmxModel> cmxModel) : cmxModel(cmxModel)
{
    renderZ = 1;
}

void MeshComponent::render(FrameInfo &frameInfo, VkPipelineLayout pipelineLayout)
{
    if (!getParent())
    {
        spdlog::error("MeshComponent: parent is null");
        return;
    };

    if (!getParent()->getVisible())
    {
        return;
    }

    if (!cmxModel)
    {
        spdlog::error("MeshComponent: missing model");
        return;
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

void MeshComponent::setModel(const std::string &name)
{
    cmxModel = getParent()->getScene()->assetsManager->getModel(name);
}

tinyxml2::XMLElement &MeshComponent::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentComponent)
{
    tinyxml2::XMLElement &componentElement = Component::save(doc, parentComponent);
    componentElement.SetAttribute("model", cmxModel->name.c_str());

    return componentElement;
}

} // namespace cmx
