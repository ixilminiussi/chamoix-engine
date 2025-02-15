#ifndef RIGID_BODY_ACTOR
#define RIGID_BODY_ACTOR

// cmx
#include <cmx/cmx_mesh_actor.h>
#include <cmx/cmx_physics_actor.h>

class RigidBodyActor : public cmx::PhysicsActor
{
  public:
    using cmx::PhysicsActor::PhysicsActor;

    void onBegin() override;
};

REGISTER_ACTOR(RigidBodyActor)

#endif
