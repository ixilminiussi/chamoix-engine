#ifndef ROTATING_ACTOR
#define ROTATING_ACTOR

#include "cmx_mesh_actor.h"

class RotatingActor : public cmx::MeshActor
{
  public:
    using cmx::MeshActor::MeshActor;

    void onBegin() override;
    void update(float dt) override;

    // for viewport
    void renderSettings() override;

    void slowdownToggle(float dt, int val);

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &, tinyxml2::XMLElement *) override;
    void load(tinyxml2::XMLElement *) override;

  private:
    float rotationSpeedSlow{0.03f};
    float rotationSpeedFast{1.f};
    float rotationSpeed{rotationSpeedFast};
};

#endif
