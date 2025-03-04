#include "cmx_billboard_component.h"

// cmx
#include "cmx_assets_manager.h"

// lib
#include <imgui.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>

namespace cmx
{

void BillboardComponent::onAttach()
{
    AssetsManager *assetsManager = getScene()->getAssetsManager();
    setDrawOption({assetsManager->getMaterial("billboard_material"), nullptr, {}});
}
// _texture->bind(frameInfo.commandBuffer, pipelineLayout);

// frameInfo.commandBuffer.draw(6, 1, 0, 0);
// }

// void BillboardComponent::setTexture(const std::string &name)
// {
//     if (getScene() != nullptr)
//     {
//         _texture = getScene()->getAssetsManager()->getTexture(name);
//     }
//     else
//     {
//         spdlog::error("MeshComponent <{0}->{1}>: Cannot setTexture before attaching to Scene object",
//                       getParent()->name.c_str(), name.c_str());
//     }
// }

// std::string BillboardComponent::getTextureName() const
// {
//     if (_texture)
//     {
//         return _texture->name;
//     }
//     return "Missing texture";
// }

void BillboardComponent::editor(int i)
{
    //     const char *selected = _texture->name.c_str();
    //     AssetsManager *assetsManager = getScene()->getAssetsManager();
    //
    //     if (ImGui::BeginCombo("Texture##", selected))
    //     {
    //         for (const auto &pair : assetsManager->getTextures())
    //         {
    //             bool isSelected = (strcmp(selected, pair.first.c_str()) == 0);
    //
    //             if (ImGui::Selectable(pair.first.c_str(), isSelected))
    //             {
    //                 selected = pair.first.c_str();
    //                 setTexture(pair.first);
    //             }
    //
    //             if (isSelected)
    //             {
    //                 ImGui::SetItemDefaultFocus();
    //             }
    //         }
    //
    //         ImGui::EndCombo();
    //     }
    //
    //     ImGui::ColorPicker3("Color", (float *)&_hue);

    Component::editor(i);
}

void BillboardComponent::load(tinyxml2::XMLElement *componentElement)
{
    Component::load(componentElement);

    try
    {
        // setTexture(componentElement->Attribute("texture"));
        // _hue.r = componentElement->FloatAttribute("r");
        // _hue.g = componentElement->FloatAttribute("g");
        // _hue.b = componentElement->FloatAttribute("b");
    }
    catch (...)
    {
    }
}

tinyxml2::XMLElement &BillboardComponent::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentComponent) const
{
    tinyxml2::XMLElement &componentElement = Component::save(doc, parentComponent);

    // componentElement.SetAttribute("texture", _texture->name.c_str());
    // componentElement.SetAttribute("r", _hue.r);
    // componentElement.SetAttribute("g", _hue.g);
    // componentElement.SetAttribute("b", _hue.b);

    return componentElement;
}

} // namespace cmx
