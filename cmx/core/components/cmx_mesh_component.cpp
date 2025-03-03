#include "cmx_mesh_component.h"

// cmx
#include "cmx_actor.h"
#include "cmx_assets_manager.h"
#include "cmx_primitives.h"

// lib
#include <GLFW/glfw3.h>
#include <cstring>
#include <glm/trigonometric.hpp>
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

MeshComponent::MeshComponent() : Drawable{&_parent}
{
}

void MeshComponent::onAttach()
{
    AssetsManager *assetsManager = getScene()->getAssetsManager();
    setDrawOption({
        assetsManager->getMaterial("shaded_material"),
        assetsManager->getModel(PRIMITIVE_CUBE),
        {assetsManager->getTexture("cmx_missing")},
    });
}

// void MeshComponent::render(const FrameInfo &frameInfo, vk::PipelineLayout pipelineLayout)
// {
// if (getParent() == nullptr)
// {
//     spdlog::critical("MeshComponent {0}: _parent is expired", name.c_str());
//     return;
// }

// if (!_textured && _texture == nullptr)
// {
//     setTexture("cmx_missing");
// }
// if (_texture == nullptr)
// {
//     spdlog::error("MeshComponent <{0}->{1}>: missing texture", getParent()->name.c_str(), name.c_str());
//     return;
// }

// if (_model == nullptr)
// {
//     spdlog::error("MeshComponent <{0}->{1}>: missing model", getParent()->name.c_str(), name.c_str());
//     return;
// }

// SimplePushConstantData push{};
// Transform transform = getWorldSpaceTransform();

// push.modelMatrix = transform.mat4();
// push.normalMatrix = transform.normalMatrix();
// push.normalMatrix[3] = glm::vec4(_color, 1.0f);

// push.normalMatrix[0][3] = _UVOffset.x;
// push.normalMatrix[1][3] = _UVOffset.y;
// push.normalMatrix[2][3] = _worldSpaceUV ? _UVScale : 0.f;
// push.normalMatrix[3][3] = _textured ? glm::radians(_UVRotate) : 100.f;

// frameInfo.commandBuffer.pushConstants(pipelineLayout,
//                                       vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0,
//                                       sizeof(SimplePushConstantData), &push);

// _model->bind(frameInfo.commandBuffer);
// _texture->bind(frameInfo.commandBuffer, pipelineLayout);

// _model->draw(frameInfo.commandBuffer);
// }

tinyxml2::XMLElement &MeshComponent::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentComponent) const
{
    tinyxml2::XMLElement &componentElement = Component::save(doc, parentComponent);

    // componentElement.SetAttribute("model", _model->name.c_str());
    // componentElement.SetAttribute("texture", _texture->name.c_str());
    // componentElement.SetAttribute("textured", _textured);
    // if (_textured)
    // {
    //     componentElement.SetAttribute("worldSpaceUV", _worldSpaceUV);
    //     if (_worldSpaceUV)
    //     {
    //         componentElement.SetAttribute("UVoffsetX", _UVOffset.x);
    //         componentElement.SetAttribute("UVoffsetY", _UVOffset.y);
    //         componentElement.SetAttribute("UVscale", _UVScale);
    //         componentElement.SetAttribute("UVrotate", _UVRotate);
    //     }
    // }
    // componentElement.SetAttribute("r", _color.r);
    // componentElement.SetAttribute("g", _color.g);
    // componentElement.SetAttribute("b", _color.b);

    return componentElement;
}

void MeshComponent::load(tinyxml2::XMLElement *componentElement)
{
    Component::load(componentElement);

    try
    {
        // setModel(componentElement->Attribute("model"));
        // setTexture(componentElement->Attribute("texture"));
        // _color.r = componentElement->FloatAttribute("r");
        // _color.g = componentElement->FloatAttribute("g");
        // _color.b = componentElement->FloatAttribute("b");
        // _textured = componentElement->BoolAttribute("textured");
        // if (_textured)
        // {
        //     _worldSpaceUV = componentElement->BoolAttribute("worldSpaceUV");
        //     if (_worldSpaceUV)
        //     {
        //         _UVOffset = glm::vec2{componentElement->FloatAttribute("UVoffsetX"),
        //                               componentElement->FloatAttribute("UVoffsetY")};
        //         _UVScale = componentElement->FloatAttribute("UVscale");
        //         _UVRotate = componentElement->FloatAttribute("UVrotate");
        //     }
        // }
    }
    catch (...)
    {
    }
}

void MeshComponent::editor(int i)
{
    // const char *selected = _model->name.c_str();
    // AssetsManager *assetsManager = getScene()->getAssetsManager();

    // if (ImGui::BeginCombo("Model##", selected))
    // {
    //     for (const auto &pair : assetsManager->getModels())
    //     {
    //         bool isSelected = (strcmp(selected, pair.first.c_str()) == 0);

    //         if (ImGui::Selectable(pair.first.c_str(), isSelected))
    //         {
    //             selected = pair.first.c_str();
    //             setModel(pair.first);
    //         }

    //         if (isSelected)
    //         {
    //             ImGui::SetItemDefaultFocus();
    //         }
    //     }

    //     ImGui::EndCombo();
    // }

    // selected = _texture->name.c_str();
    // assetsManager = getScene()->getAssetsManager();

    // ImGui::Checkbox("use texture", &_textured);

    // if (_textured)
    // {
    //     if (ImGui::BeginCombo("Texture##", selected))
    //     {
    //         for (const auto &pair : assetsManager->getTextures())
    //         {
    //             bool isSelected = (strcmp(selected, pair.first.c_str()) == 0);

    //             if (ImGui::Selectable(pair.first.c_str(), isSelected))
    //             {
    //                 selected = pair.first.c_str();
    //                 setTexture(pair.first);
    //             }

    //             if (isSelected)
    //             {
    //                 ImGui::SetItemDefaultFocus();
    //             }
    //         }

    //         ImGui::EndCombo();
    //     }

    //     ImGui::Checkbox("World space UV", &_worldSpaceUV);
    //     if (_worldSpaceUV)
    //     {
    //         ImGui::SliderFloat2("UV offset", (float *)&_UVOffset, -1.f, 1.f);
    //         ImGui::DragFloat("Scale", &_UVScale);
    //         ImGui::DragFloat("Rotate", &_UVRotate, 1.f, -180.f, 180.f);
    //     }
    // }

    // ImGui::ColorEdit3("Color##", (float *)&_color);

    Component::editor(i);
}

} // namespace cmx
