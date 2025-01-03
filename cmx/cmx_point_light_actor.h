#ifndef CMX_POINT_LIGHT_ACTOR
#define CMX_POINT_LIGHT_ACTOR

#include "cmx_actor.h"

namespace cmx
{

class PointLightActor : public Actor
{
  public:
    using Actor::Actor;

    void onBegin() override;
    void update(float dt) override;

  private:
    std::shared_ptr<class PointLightComponent> _pointLightComponent;
};

} // namespace cmx

#endif
