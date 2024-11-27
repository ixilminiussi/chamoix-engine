#pragma once

#include "cmx_actor.h"

class TriangleActor : public cmx::Actor
{
    using Actor::Actor;

  public:
    void onBegin() override;
    void update(float dt) override;
};
