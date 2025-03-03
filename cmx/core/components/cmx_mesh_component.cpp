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
    Drawable::editor(i);
    Component::editor(i);
}

} // namespace cmx
