#pragma once

#include "cmx_actor.h"

namespace cmx
{

enum CmxPrimitives
{
    SPHERE,
    BOX,
    // PLANE,
    // TORUS,
    // CYLINDER,
    // CONE,
    // CAPSULE,
};

class CmxMeshActor : public Actor
{
  public:
    using Actor::Actor;

    void onBegin() override;
    void update(float dt) override;

    // for viewport
    void renderSettings() override;

    void updateMesh(CmxPrimitives);
    void updateMesh(std::string &assetName);
};

} // namespace cmx
