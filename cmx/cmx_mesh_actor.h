#ifndef CMX_MESH_ACTOR
#define CMX_MESH_ACTOR

#include "cmx_actor.h"

namespace cmx
{

enum Primitives
{
    SPHERE,
    BOX,
    // PLANE,
    // TORUS,
    // CYLINDER,
    // CONE,
    // CAPSULE,
};

class MeshActor : public Actor
{
  public:
    using Actor::Actor;

    void onBegin() override;
    void update(float dt) override;

    // for viewport
    void renderSettings() override;

    void updateMesh(Primitives){};
    void updateMesh(std::string &assetName);

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &, tinyxml2::XMLElement *) override;
    void load(tinyxml2::XMLElement *) override;

  private:
    std::shared_ptr<class MeshComponent> _meshComponent;
};

} // namespace cmx

#endif
