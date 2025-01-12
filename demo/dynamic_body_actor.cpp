#include "dynamic_body_actor.h"

// cmx
#include <cmx/cmx_physics_component.h>
#include <cmx/cmx_shapes.h>

void DynamicBodyActor::onBegin()
{
    cmx::PhysicsActor::onBegin();

    _physicsComponent->setDynamic();
    _physicsComponent->setShape(new cmx::CmxSphere{glm::vec3{}, 1.0f, this});
}
