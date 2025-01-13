#include "wall_actor.h"

// cmx
#include <cmx/cmx_mesh_component.h>
#include <cmx/cmx_physics_component.h>
#include <cmx/cmx_primitives.h>

void WallActor::onBegin()
{
    cmx::PhysicsActor::onBegin();

    _physicsComponent->setStatic();
    _physicsComponent->setShape(PRIMITIVE_CUBE);

    std::shared_ptr<cmx::MeshComponent> _meshComponent = std::make_shared<cmx::MeshComponent>();
    attachComponent(_meshComponent);
    _meshComponent->setModel(PRIMITIVE_CUBE);
}
