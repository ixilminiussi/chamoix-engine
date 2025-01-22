#include "cmx_mesh_component.h"

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
    if (!_cmxModel)
    {
        setModel(PRIMITIVE_CUBE);
    }
}

void MeshComponent::render(const FrameInfo &frameInfo, VkPipelineLayout pipelineLayout)
{
    if (getParent() == nullptr)
    {
        spdlog::critical("MeshComponent: _parent is expired");
        return;
    }

    if (!_cmxModel)
    {
        spdlog::error("MeshComponent: missing model");
        return;
    }

    SimplePushConstantData push{};
    Transform transform = getAbsoluteTransform();

    push.modelMatrix = transform.mat4();
    push.normalMatrix = transform.normalMatrix();
    push.normalMatrix[3] = glm::vec4(_color, 1.0f);

    vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData),
                       &push);

    _cmxModel->bind(frameInfo.commandBuffer);
    _cmxModel->draw(frameInfo.commandBuffer);
}

void MeshComponent::setModel(const std::string &name)
{
    if (getScene() != nullptr)
    {
        _cmxModel = getScene()->getAssetsManager()->getModel(name);
    }
    else
    {
        spdlog::error("MeshComponent: Cannot setModel before attaching to Scene object");
    }
}

std::string MeshComponent::getModelName()
{
    if (_cmxModel)
    {
        return _cmxModel->name;
    }
    return "Missing model";
}

tinyxml2::XMLElement &MeshComponent::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentComponent)
{
    tinyxml2::XMLElement &componentElement = Component::save(doc, parentComponent);
    componentElement.SetAttribute("model", _cmxModel->name.c_str());
    componentElement.SetAttribute("r", _color.r);
    componentElement.SetAttribute("g", _color.g);
    componentElement.SetAttribute("b", _color.b);

    return componentElement;
}

void MeshComponent::load(tinyxml2::XMLElement *componentElement)
{
    Component::load(componentElement);

    setModel(componentElement->Attribute("model"));
    _color.r = componentElement->FloatAttribute("r");
    _color.g = componentElement->FloatAttribute("g");
    _color.b = componentElement->FloatAttribute("b");
}

void MeshComponent::editor(int i)
{
    const char *selected = _cmxModel->name.c_str();
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
