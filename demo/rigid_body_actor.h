#ifndef RIGID_BODY_ACTOR
#define RIGID_BODY_ACTOR

#include <cmx/cmx_physics_actor.h>

class RigidBodyActor : public cmx::PhysicsActor
{
  public:
    using cmx::PhysicsActor::PhysicsActor;

    void onBegin() override;
};

#endif
