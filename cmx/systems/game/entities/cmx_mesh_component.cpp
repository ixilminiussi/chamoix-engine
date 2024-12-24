#include "cmx_mesh_component.h"

#include "cmx_actor.h"
#include "cmx_assets_manager.h"
#include "cmx_frame_info.h"
#include "cmx_model.h"
#include "cmx_render_system.h"
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
    _renderZ = 1;
}

MeshComponent::MeshComponent(std::shared_ptr<CmxModel> cmxModel) : _cmxModel(cmxModel)
{
    _renderZ = 1;
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

    if (!_cmxModel)
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

    _cmxModel->bind(frameInfo.commandBuffer);
    _cmxModel->draw(frameInfo.commandBuffer);
}

void MeshComponent::setModel(const std::string &name)
{
    _cmxModel = getParent()->getScene()->_assetsManager->getModel(name);
}

tinyxml2::XMLElement &MeshComponent::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentComponent)
{
    tinyxml2::XMLElement &componentElement = Component::save(doc, parentComponent);
    componentElement.SetAttribute("model", _cmxModel->name.c_str());

    return componentElement;
}

} // namespace cmx
