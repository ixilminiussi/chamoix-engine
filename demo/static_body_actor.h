#ifndef STATIC_BODY_ACTOR
#define STATIC_BODY_ACTOR

// cmx
#include <cmx/cmx_mesh_actor.h>
#include <cmx/cmx_physics_actor.h>

class StaticBodyActor : public cmx::PhysicsActor
{
  public:
    using cmx::PhysicsActor::PhysicsActor;

    void onBegin() override;
};

#endif
