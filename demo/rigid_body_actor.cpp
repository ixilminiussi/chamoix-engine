#include "rigid_body_actor.h"

// cmx
#include <cmx/cmx_mesh_component.h>
#include <cmx/cmx_physics_actor.h>
#include <cmx/cmx_physics_component.h>

void RigidBodyActor::onBegin()
{
    cmx::PhysicsActor::onBegin();

    _physicsComponent->setRigid();
    attachComponent(std::make_shared<cmx::MeshComponent>());
}
