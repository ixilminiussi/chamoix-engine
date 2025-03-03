#ifndef ROOM_ACTOR
#define ROOM_ACTOR

#include <cmx_mesh_actor.h>
#include <cmx_mesh_component.h>
#include <cmx_physics_actor.h>
#include <cmx_physics_component.h>

class RoomActor : public cmx::Actor
{
  public:
    using cmx::Actor::Actor;

    void onBegin() override;

  protected:
    std::shared_ptr<class cmx::MeshComponent> _meshComponent;
    std::shared_ptr<class cmx::PhysicsComponent> _floorCollision;
    std::shared_ptr<class cmx::PhysicsComponent> _ceilingCollision;
    std::shared_ptr<class cmx::PhysicsComponent> _wall1Collision;
    std::shared_ptr<class cmx::PhysicsComponent> _wall2Collision;
    std::shared_ptr<class cmx::PhysicsComponent> _wall3Collision;
    std::shared_ptr<class cmx::PhysicsComponent> _wall4Collision;
};

REGISTER_ACTOR(RoomActor)

#endif
