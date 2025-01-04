#include "cmx_point_light_component.h"

// cmx
#include "cmx_actor.h"
#include "cmx_billboard_render_system.h"
#include "cmx_frame_info.h"
#include "cmx_graphics_manager.h"
#include "cmx_render_system.h"

// lib
#include "imgui.h"
#include <vulkan/vulkan_core.h>

namespace cmx
{

uint32_t PointLightComponent::_keyChain{0u};

PointLightComponent::PointLightComponent()
{
    _renderZ = TRANSPARENT_BILLBOARD_Z;
    _requestedRenderSystem = BILLBOARD_RENDER_SYSTEM;
}

void PointLightComponent::update(float dt)
{
    Transform absoluteTransform = getAbsoluteTransform();
    _absolutePosition = absoluteTransform.position;
    _absoluteScaleXY = glm::vec2(absoluteTransform.scale.x, absoluteTransform.scale.y);
}

void PointLightComponent::onAttach()
{
    _key = _keyChain++;
    getScene()->getGraphicsManager()->addPointLight(_key, {&_absolutePosition, &_lightColor, &_lightIntensity});
}

void PointLightComponent::onDetach()
{
    getScene()->getGraphicsManager()->removePointLight(_key);
}

void PointLightComponent::render(FrameInfo &frameInfo, VkPipelineLayout pipelineLayout)
{
    if (getParent() == nullptr)
    {
        spdlog::critical("MeshComponent: _parent is expired");
        return;
    }

    BillboardPushConstant pushConstant;
    pushConstant.position = glm::vec4(_absolutePosition, 1.0f);
    pushConstant.color = glm::vec4(_lightColor, _lightIntensity);
    pushConstant.scale = _absoluteScaleXY;

    vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(BillboardPushConstant),
                       &pushConstant);

    vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
}

void PointLightComponent::editor(int i)
{
    ImGui::DragFloat("Light Intensity", &_lightIntensity, 0.01f, 0.0f, 10.0f, "%.2f");
    ImGui::ColorPicker3("Light Color", (float *)&_lightColor);

    Component::editor(i);
}

void PointLightComponent::load(tinyxml2::XMLElement *componentElement)
{
    Component::load(componentElement);

    _lightIntensity = componentElement->FloatAttribute("lightIntensity");
    _lightColor.r = componentElement->FloatAttribute("r");
    _lightColor.g = componentElement->FloatAttribute("g");
    _lightColor.b = componentElement->FloatAttribute("b");
}

tinyxml2::XMLElement &PointLightComponent::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentComponent)
{
    tinyxml2::XMLElement &componentElement = Component::save(doc, parentComponent);

    componentElement.SetAttribute("lightIntensity", _lightIntensity);
    componentElement.SetAttribute("r", _lightColor.r);
    componentElement.SetAttribute("g", _lightColor.g);
    componentElement.SetAttribute("b", _lightColor.b);

    return componentElement;
}

} // namespace cmx
