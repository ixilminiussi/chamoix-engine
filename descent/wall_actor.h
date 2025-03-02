#ifndef WALL_ACTOR
#define WALL_ACTOR

#include <cmx_mesh_actor.h>
#include <cmx_physics_actor.h>

class WallActor : public cmx::PhysicsActor
{
  public:
    using cmx::PhysicsActor::PhysicsActor;

    void onBegin() override;

  protected:
    std::shared_ptr<class cmx::MeshComponent> _meshComponent;
};

REGISTER_ACTOR(WallActor)

#endif
