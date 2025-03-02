#include "static_body_actor.h"

// cmx
#include <cmx_physics_component.h>
#include <cmx_primitives.h>
#include <cmx_shapes.h>

void StaticBodyActor::onBegin()
{
    cmx::PhysicsActor::onBegin();

    _physicsComponent->setPhysicsMode(cmx::PhysicsMode::STATIC);
    _physicsComponent->setShape(PRIMITIVE_CUBE);
}
