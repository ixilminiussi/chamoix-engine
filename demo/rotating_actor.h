#ifndef ROTATING_ACTOR
#define ROTATING_ACTOR

#include <cmx_actor.h>
#include <cmx_register.h>

class RotatingActor : public cmx::Actor
{
  public:
    using cmx::Actor::Actor;

    void onBegin() override;
    void update(float dt) override;

    void editor() override;

    void slowdownToggle(float dt, int val);

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &, tinyxml2::XMLElement *) const override;
    void load(tinyxml2::XMLElement *) override;

  private:
    float rotationSpeedSlow{0.03f};
    float rotationSpeedFast{1.f};
    float rotationSpeed{rotationSpeedFast};
};

REGISTER_ACTOR(RotatingActor)

#endif
