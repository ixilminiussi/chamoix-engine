#include "cmx_physics_component.h"

// cmx
#include "cmx_physics_body.h"
#include "cmx_physics_manager.h"

// lib
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/quaternion.hpp>

namespace cmx
{

PhysicsComponent::PhysicsComponent() : PhysicsBody{&_parent}
{
}

void PhysicsComponent::onDetach()
{
}

void PhysicsComponent::onAttach()
{
    getScene()->getPhysicsManager()->add(this);
}

tinyxml2::XMLElement &PhysicsComponent::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const
{
    tinyxml2::XMLElement &componentElement = Component::save(doc, parentElement);
    PhysicsBody::save(*parentElement);

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
}

} // namespace cmx
