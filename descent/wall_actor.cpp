#include "wall_actor.h"

// cmx
#include <cmx_mesh_component.h>
#include <cmx_physics.h>
#include <cmx_physics_component.h>
#include <cmx_primitives.h>

void WallActor::onBegin()
{
    cmx::PhysicsActor::onBegin();

    _physicsComponent->setPhysicsMode(cmx::PhysicsMode::STATIC);
    _physicsComponent->setShape(PRIMITIVE_CUBE);

    std::shared_ptr<cmx::MeshComponent> _meshComponent = std::make_shared<cmx::MeshComponent>();
    attachComponent(_meshComponent);
}
