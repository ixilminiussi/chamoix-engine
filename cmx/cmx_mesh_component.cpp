#include "cmx_mesh_component.h"

#include "cmx/cmx_texture.h"
#include "cmx_actor.h"
#include "cmx_assets_manager.h"
#include "cmx_frame_info.h"
#include "cmx_model.h"
#include "cmx_primitives.h"
#include "cmx_render_system.h"
#include "cmx_shaded_render_system.h"

// lib
#include <GLFW/glfw3.h>
#include <cstring>
#include <imgui.h>
#include <spdlog/spdlog.h>
#include <tinyxml2.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace cmx
{

MeshComponent::MeshComponent()
{
    _renderZ = DEFAULT_Z;
    _requestedRenderSystem = SHADED_RENDER_SYSTEM;
}

void MeshComponent::onAttach()
{
    if (!_model)
    {
        setModel(PRIMITIVE_CUBE);
    }
    if (!_texture)
    {
        setTexture("missing");
    }
}

void MeshComponent::render(const FrameInfo &frameInfo, vk::PipelineLayout pipelineLayout)
{
    if (getParent() == nullptr)
    {
        spdlog::critical("MeshComponent {0}: _parent is expired", name.c_str());
        return;
    }

    if (_model == nullptr)
    {
        spdlog::error("MeshComponent <{0}->{1}>: missing model", getParent()->name.c_str(), name.c_str());
        return;
    }

    if (_texture == nullptr)
    {
        spdlog::error("MeshComponent <{0}->{1}>: missing texture", getParent()->name.c_str(), name.c_str());
        return;
    }

    SimplePushConstantData push{};
    Transform transform = getAbsoluteTransform();

    push.modelMatrix = transform.mat4();
    push.normalMatrix = transform.normalMatrix();
    push.normalMatrix[3] = glm::vec4(_color, 1.0f);

    frameInfo.commandBuffer.pushConstants(pipelineLayout,
                                          vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0,
                                          sizeof(SimplePushConstantData), &push);

    _model->bind(frameInfo.commandBuffer);
    _texture->bind(frameInfo.commandBuffer, pipelineLayout);

    _model->draw(frameInfo.commandBuffer);
}

void MeshComponent::setModel(const std::string &name)
{
    if (getScene() != nullptr)
    {
        _model = getScene()->getAssetsManager()->getModel(name);
    }
    else
    {
        spdlog::error("MeshComponent: Cannot setModel before attaching to Scene object");
    }
}

void MeshComponent::setTexture(const std::string &name)
{
    if (getScene() != nullptr)
    {
        _texture = getScene()->getAssetsManager()->getTexture(name);
    }
    else
    {
        spdlog::error("MeshComponent: Cannot setTexture before attaching to Scene object");
    }
}

void MeshComponent::setColor(const glm::vec3 &color)
{
    _color = color;
}

std::string MeshComponent::getModelName() const
{
    if (_model != nullptr)
    {
        return _model->name;
    }
    return "Missing model";
}

std::string MeshComponent::getTextureName() const
{
    if (_texture != nullptr)
    {
        return _texture->name;
    }
    return "Missing texture";
}

tinyxml2::XMLElement &MeshComponent::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentComponent)
{
    tinyxml2::XMLElement &componentElement = Component::save(doc, parentComponent);

    componentElement.SetAttribute("model", _model->name.c_str());
    componentElement.SetAttribute("texture", _texture->name.c_str());
    componentElement.SetAttribute("r", _color.r);
    componentElement.SetAttribute("g", _color.g);
    componentElement.SetAttribute("b", _color.b);

    return componentElement;
}

void MeshComponent::load(tinyxml2::XMLElement *componentElement)
{
    Component::load(componentElement);

    setModel(componentElement->Attribute("model"));
    // setTexture(componentElement->Attribute("texture"));
    _color.r = componentElement->FloatAttribute("r");
    _color.g = componentElement->FloatAttribute("g");
    _color.b = componentElement->FloatAttribute("b");
}

void MeshComponent::editor(int i)
{
    const char *selected = _model->name.c_str();
    AssetsManager *assetsManager = getScene()->getAssetsManager();

    if (ImGui::BeginCombo("Model##", selected))
    {
        for (const auto &pair : assetsManager->getModels())
        {
            bool isSelected = (strcmp(selected, pair.first.c_str()) == 0);

            if (ImGui::Selectable(pair.first.c_str(), isSelected))
            {
                selected = pair.first.c_str();
                setModel(pair.first);
            }

            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

    ImGui::ColorEdit3("Color##", (float *)&_color);

    Component::editor(i);
}

} // namespace cmx
