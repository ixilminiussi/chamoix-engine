#include "cmx_physics_component.h"

// cmx
#include "IconsMaterialSymbols.h"
#include "cmx_physics_body.h"
#include "cmx_physics_manager.h"
#include "cmx_primitives.h"
#include "imgui.h"

// lib
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/quaternion.hpp>

namespace cmx
{

PhysicsComponent::PhysicsComponent()
    : PhysicsBody{&_parent}
#ifndef NDEBUG
      ,
      Drawable{&_parent}
#endif
{
}

void PhysicsComponent::onDetach()
{
}

void PhysicsComponent::onAttach()
{
    getScene()->getPhysicsManager()->add(this);

#ifndef NDEBUG
    AssetsManager *assetsManager = getScene()->getAssetsManager();
    setDrawOption({
        assetsManager->getMaterial("mesh_material"),
        assetsManager->getModel(PRIMITIVE_SPHERE),
        {},
    });
#endif
}

void PhysicsComponent::setPhysicsMode(PhysicsMode mode)
{
    PhysicsBody::setPhysicsMode(mode);
}

void PhysicsComponent::setShape(const std::string &type)
{
    PhysicsBody::setShape(type);

    setModel(type.c_str());
}

tinyxml2::XMLElement &PhysicsComponent::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const
{
    tinyxml2::XMLElement &componentElement = Component::save(doc, parentElement);
    PhysicsBody::save(componentElement);

    return componentElement;
}

void PhysicsComponent::load(tinyxml2::XMLElement *componentElement)
{
    Component::load(componentElement);
    PhysicsBody::load(componentElement);
}

void PhysicsComponent::editor(int i)
{
    PhysicsBody::editor(i);
    Component::editor(i);

    ImGui::Checkbox(ICON_MS_VISIBILITY, &_isVisible);
}

} // namespace cmx
