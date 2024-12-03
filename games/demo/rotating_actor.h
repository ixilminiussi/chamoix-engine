#pragma once

#include "cmx_actor.h"

class RotatingActor : public cmx::Actor
{
  public:
    using cmx::Actor::Actor;

    void onBegin() override;
    void update(float dt) override;

    // for viewport
    void renderSettings() override;

    void slowdownToggle(float dt, int val);

  private:
    float rotationSpeedSlow{0.03f};
    float rotationSpeedFast{1.f};
    float rotationSpeed{rotationSpeedFast};
};
