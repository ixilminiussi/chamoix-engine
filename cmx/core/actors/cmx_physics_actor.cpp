#include "cmx_physics_actor.h"

// cmx
#include "cmx_physics_component.h"
#include "cmx_primitives.h"

// lib
#include <glm/ext/scalar_constants.hpp>
#include <glm/geometric.hpp>

namespace cmx
{

void PhysicsActor::onBegin()
{
    Actor::onBegin();

    _physicsComponent = std::make_shared<PhysicsComponent>();
    attachComponent(_physicsComponent);
    _physicsComponent->setShape(PRIMITIVE_SPHERE);
}

} // namespace cmx
