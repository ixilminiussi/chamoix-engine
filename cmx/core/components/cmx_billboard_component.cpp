#include "cmx_billboard_component.h"

// cmx
#include "cmx_assets_manager.h"
#include "cmx_billboard_material.h"
#include "cmx_material.h"

// lib
#include <imgui.h>
#include <memory>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>

namespace cmx
{

void BillboardComponent::onAttach()
{
    AssetsManager *assetsManager = getScene()->getAssetsManager();

    Material *material = assetsManager->getMaterial("billboard_material");
    setDrawOption({material, nullptr, {}});

    _material = static_cast<BillboardMaterial *>(material);
}

void BillboardComponent::editor(int i)
{
    Component::editor(i);
    Drawable::editor(i);
}

void BillboardComponent::setHue(const glm::vec4 hue)
{
    _material->setHue(hue);
}

void BillboardComponent::load(tinyxml2::XMLElement *componentElement)
{
    Component::load(componentElement);
}

tinyxml2::XMLElement &BillboardComponent::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const
{
    tinyxml2::XMLElement &componentElement = Component::save(doc, parentElement);

    return componentElement;
}

} // namespace cmx
