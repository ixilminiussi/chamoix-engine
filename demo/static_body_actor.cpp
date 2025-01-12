#include "static_body_actor.h"
#include "cmx/cmx_primitives.h"

// cmx
#include <cmx/cmx_physics_component.h>
#include <cmx/cmx_shapes.h>

void StaticBodyActor::onBegin()
{
    cmx::PhysicsActor::onBegin();

    _physicsComponent->setStatic();
    _physicsComponent->setShape(PRIMITIVE_CUBE);
}
