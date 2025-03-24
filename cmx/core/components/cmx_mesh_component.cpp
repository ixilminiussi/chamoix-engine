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
        assetsManager->getMaterial("void_material"),
        assetsManager->getModel(PRIMITIVE_CUBE),
        {assetsManager->get2DTexture("cmx_missing")},
    });
}

tinyxml2::XMLElement &MeshComponent::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const
{
    tinyxml2::XMLElement &componentElement = Component::save(doc, parentElement);
    Drawable::save(doc, &componentElement);

    return componentElement;
}

void MeshComponent::load(tinyxml2::XMLElement *componentElement)
{
    Component::load(componentElement);
    Drawable::load(componentElement);
}

void MeshComponent::editor(int i)
{
    Component::editor(i);
    Drawable::editor(i);
}

} // namespace cmx
