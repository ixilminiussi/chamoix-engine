#include "cmx_point_light_component.h"

// cmx
#include "cmx/cmx_assets_manager.h"
#include "cmx/cmx_texture.h"
#include "cmx_billboard_render_system.h"
#include "cmx_frame_info.h"
#include "cmx_graphics_manager.h"
#include "cmx_render_system.h"

// lib
#include <imgui.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>

namespace cmx
{

uint32_t PointLightComponent::_keyChain{0u};

PointLightComponent::PointLightComponent()
{
    _renderZ = TRANSPARENT_BILLBOARD_Z;
    _requestedRenderSystem = BILLBOARD_RENDER_SYSTEM;
}

void PointLightComponent::onAttach()
{
    if (_texture == nullptr)
    {
        _texture = getScene()->getAssetsManager()->getTexture("cmx_point_light");
    }

    _key = _keyChain++;

    Transform absoluteTransform = getAbsoluteTransform();
    _absolutePosition = absoluteTransform.position;
    _absoluteScaleXY = glm::vec2(absoluteTransform.scale.x, absoluteTransform.scale.y);

    getScene()->getGraphicsManager()->addPointLight(_key, {&_absolutePosition, &_lightColor, &_lightIntensity});
}

void PointLightComponent::onDetach()
{
    getScene()->getGraphicsManager()->removePointLight(_key);
}

void PointLightComponent::render(const FrameInfo &frameInfo, vk::PipelineLayout pipelineLayout)
{
    if (getParent() == nullptr)
    {
        spdlog::critical("MeshComponent <{0}>: _parent is expired", name.c_str());
        return;
    }

    if (_texture == nullptr)
    {
        spdlog::error("MeshComponent <{0}->{1}>: missing texture", getParent()->name.c_str(), name.c_str());
        return;
    }

    Transform transform = getAbsoluteTransform();
    _absolutePosition = transform.position;
    _absoluteScaleXY = glm::vec2(transform.scale.x, transform.scale.y);

    BillboardPushConstant pushConstant;
    pushConstant.position = glm::vec4(_absolutePosition, 1.0f);
    pushConstant.color = glm::vec4(_lightColor, _lightIntensity);
    pushConstant.scale = _absoluteScaleXY;

    frameInfo.commandBuffer.pushConstants(pipelineLayout,
                                          vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0,
                                          sizeof(BillboardPushConstant), &pushConstant);

    _texture->bind(frameInfo.commandBuffer, pipelineLayout);

    frameInfo.commandBuffer.draw(6, 1, 0, 0);
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
