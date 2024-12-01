#pragma once

#include "cmx_actor.h"

class RotatingActor : public cmx::Actor
{
  public:
    using cmx::Actor::Actor;

    void onBegin() override;
    void update(float dt) override;

    void slowdownOn(float dt);
    void slowdownOff(float dt);

  private:
    float rotationSpeedSlow{0.03f}, rotationSpeedFast{1.f}, rotationSpeed{rotationSpeedFast};
};
