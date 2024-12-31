#ifndef CMX_MESH_ACTOR
#define CMX_MESH_ACTOR

#include "cmx_actor.h"

namespace cmx
{

class MeshActor : public Actor
{
  public:
    using Actor::Actor;

    void onBegin() override;
    void update(float dt) override;

    // for viewport
    void editor() override;

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &, tinyxml2::XMLElement *) override;
    void load(tinyxml2::XMLElement *) override;

  private:
    std::shared_ptr<class MeshComponent> _meshComponent;
};

} // namespace cmx

#endif
