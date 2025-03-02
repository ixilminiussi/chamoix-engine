#ifndef DYNAMIC_BODY_ACTOR
#define DYNAMIC_BODY_ACTOR

// cmx
#include <cmx_mesh_actor.h>
#include <cmx_physics_actor.h>

class DynamicBodyActor : public cmx::PhysicsActor
{
  public:
    using cmx::PhysicsActor::PhysicsActor;

    virtual void onBegin() override;
};

REGISTER_ACTOR(DynamicBodyActor)

#endif
