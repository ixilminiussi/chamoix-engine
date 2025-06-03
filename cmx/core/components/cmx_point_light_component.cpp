#include "cmx_point_light_component.h"

// cmx
#include "cmx_assets_manager.h"
#include "cmx_billboard_component.h"
#include "cmx_billboard_material.h"
#include "cmx_frame_info.h"
#include "cmx_graphics_manager.h"
#include "cmx_render_system.h"
#include "cmx_texture.h"

// lib
#include <imgui.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>

namespace cmx
{

uint32_t PointLightComponent::_keyChain{0u};

PointLightComponent::PointLightComponent() : Drawable{&_parent}
{
}

void PointLightComponent::onAttach()
{
    BillboardComponent::onAttach();
    setTextures({"cmx_point_light"});

    _key = _keyChain++;

    Transform absoluteTransform = getWorldSpaceTransform();
    _absolutePosition = absoluteTransform.position;
    _absoluteScaleXY = glm::vec2(absoluteTransform.scale.x, absoluteTransform.scale.y);

    getScene()->getLightEnvironment()->addPointLight(_key, {&_absolutePosition, &_lightColor, &_lightIntensity});
}

void PointLightComponent::onDetach()
{
    getScene()->getLightEnvironment()->removePointLight(_key);
}

void PointLightComponent::update(float dt)
{
    Transform transform = getWorldSpaceTransform();
    _absolutePosition = transform.position;
    _absoluteScaleXY = glm::vec2(transform.scale.x, transform.scale.y);
    _material->setHue(glm::vec4(_lightColor, 1.0));
}

void PointLightComponent::editor(int i)
{
    ImGui::DragFloat("Light Intensity", &_lightIntensity, 0.01f, 0.0f, 100.0f, "%.2f");
    ImGui::ColorPicker3("Light Color", (float *)&_lightColor);
    _material->setHue(glm::vec4(_lightColor, 1.0));

    BillboardComponent::editor(i);
}

void PointLightComponent::load(tinyxml2::XMLElement *componentElement)
{
    BillboardComponent::load(componentElement);

    _lightIntensity = componentElement->FloatAttribute("lightIntensity");
    _lightColor.r = componentElement->FloatAttribute("r");
    _lightColor.g = componentElement->FloatAttribute("g");
    _lightColor.b = componentElement->FloatAttribute("b");
    _material->setHue(glm::vec4(_lightColor, 1.0));

    Transform absoluteTransform = getWorldSpaceTransform();
    _absolutePosition = absoluteTransform.position;
    _absoluteScaleXY = glm::vec2(absoluteTransform.scale.x, absoluteTransform.scale.y);
}

tinyxml2::XMLElement &PointLightComponent::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const
{
    tinyxml2::XMLElement &componentElement = BillboardComponent::save(doc, parentElement);

    componentElement.SetAttribute("lightIntensity", _lightIntensity);
    componentElement.SetAttribute("r", _lightColor.r);
    componentElement.SetAttribute("g", _lightColor.g);
    componentElement.SetAttribute("b", _lightColor.b);

    return componentElement;
}

} // namespace cmx
