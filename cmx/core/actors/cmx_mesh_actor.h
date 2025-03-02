#ifndef CMX_MESH_ACTOR
#define CMX_MESH_ACTOR

#include "cmx_actor.h"
#include "cmx_register.h"

namespace cmx
{

class MeshActor : public Actor
{
  public:
    using Actor::Actor;

    void onBegin() override;

  protected:
    std::shared_ptr<class MeshComponent> _meshComponent;
};

} // namespace cmx

REGISTER_ACTOR(cmx::MeshActor)

#endif
