#include "room_actor.h"

// cmx
#include <cmx_mesh_component.h>
#include <cmx_physics.h>
#include <cmx_physics_component.h>
#include <cmx_primitives.h>

void RoomActor::onBegin()
{
    Actor::onBegin();

    _floorCollision = std::make_shared<cmx::PhysicsComponent>();
    attachComponent(_floorCollision, "FloorCollision");
    _floorCollision->setShape(PRIMITIVE_PLANE);
    _ceilingCollision = std::make_shared<cmx::PhysicsComponent>();
    attachComponent(_ceilingCollision, "CeilingCollision");
    _ceilingCollision->setShape(PRIMITIVE_PLANE);
    _wall1Collision = std::make_shared<cmx::PhysicsComponent>();
    attachComponent(_wall1Collision, "Wall1Collision");
    _wall1Collision->setShape(PRIMITIVE_PLANE);
    _wall2Collision = std::make_shared<cmx::PhysicsComponent>();
    attachComponent(_wall2Collision, "Wall2Collision");
    _wall2Collision->setShape(PRIMITIVE_PLANE);
    _wall3Collision = std::make_shared<cmx::PhysicsComponent>();
    attachComponent(_wall3Collision, "Wall3Collision");
    _wall3Collision->setShape(PRIMITIVE_PLANE);
    _wall4Collision = std::make_shared<cmx::PhysicsComponent>();
    attachComponent(_wall4Collision, "Wall4Collision");
    _wall4Collision->setShape(PRIMITIVE_PLANE);

    _meshComponent = std::make_shared<cmx::MeshComponent>();
    attachComponent(_meshComponent);
    _meshComponent->setModel(PRIMITIVE_CONTAINER);
}
