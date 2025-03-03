#include "dynamic_body_actor.h"

// cmx
#include <cmx_physics.h>
#include <cmx_physics_component.h>
#include <cmx_primitives.h>
#include <cmx_shapes.h>

void DynamicBodyActor::onBegin()
{
    cmx::PhysicsActor::onBegin();

    _physicsComponent->setPhysicsMode(cmx::PhysicsMode::DYNAMIC);
    _physicsComponent->setShape(PRIMITIVE_SPHERE);
}
