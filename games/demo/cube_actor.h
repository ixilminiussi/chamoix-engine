#pragma once

#include "cmx_actor.h"

class CubeActor : public cmx::Actor
{
  public:
    using cmx::Actor::Actor;

    void onBegin() override;
    void update(float dt) override;
};
