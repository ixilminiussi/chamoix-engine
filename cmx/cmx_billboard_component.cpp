#include "cmx_billboard_component.h"

// cmx
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

BillboardComponent::BillboardComponent()
{
    _renderZ = TRANSPARENT_BILLBOARD_Z;
    _requestedRenderSystem = BILLBOARD_RENDER_SYSTEM;
}

void BillboardComponent::render(const FrameInfo &frameInfo, vk::PipelineLayout pipelineLayout)
{
    if (getParent() == nullptr)
    {
        spdlog::critical("MeshComponent: _parent is expired");
        return;
    }

    Transform transform = getAbsoluteTransform();

    BillboardPushConstant pushConstant;
    pushConstant.position = glm::vec4(transform.position, 1.0f);
    pushConstant.color = glm::vec4(_hue, 1.f);
    pushConstant.scale = glm::vec2(transform.scale.x, transform.scale.y);

    frameInfo.commandBuffer.pushConstants(pipelineLayout,
                                          vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0,
                                          sizeof(BillboardPushConstant), &pushConstant);

    frameInfo.commandBuffer.draw(6, 1, 0, 0);
}

void BillboardComponent::editor(int i)
{
    ImGui::ColorPicker3("Color", (float *)&_hue);

    Component::editor(i);
}

void BillboardComponent::load(tinyxml2::XMLElement *componentElement)
{
    Component::load(componentElement);

    _hue.r = componentElement->FloatAttribute("r");
    _hue.g = componentElement->FloatAttribute("g");
    _hue.b = componentElement->FloatAttribute("b");
}

tinyxml2::XMLElement &BillboardComponent::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentComponent)
{
    tinyxml2::XMLElement &componentElement = Component::save(doc, parentComponent);

    componentElement.SetAttribute("r", _hue.r);
    componentElement.SetAttribute("g", _hue.g);
    componentElement.SetAttribute("b", _hue.b);

    return componentElement;
}

} // namespace cmx
