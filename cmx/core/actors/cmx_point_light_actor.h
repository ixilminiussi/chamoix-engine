#ifndef CMX_POINT_LIGHT_ACTOR
#define CMX_POINT_LIGHT_ACTOR

#include "cmx_actor.h"
#include "cmx_point_light_component.h"
#include "cmx_register.h"

namespace cmx
{

class PointLightActor : public Actor
{
  public:
    using Actor::Actor;

    void onBegin() override;
    void update(float dt) override;

    float getLightIntensity() const
    {
        return _pointLightComponent->getLightIntensity();
    }
    void setLightIntensity(float lightIntensity)
    {
        _pointLightComponent->setLightIntensity(lightIntensity);
    }

    const glm::vec3 &getLightColor()
    {
        return _pointLightComponent->getLightColor();
    }
    void setLightColor(const glm::vec3 &lightColor)
    {
        _pointLightComponent->setLightColor(lightColor);
    }

  private:
    std::shared_ptr<PointLightComponent> _pointLightComponent;
};

} // namespace cmx

REGISTER_ACTOR(cmx::PointLightActor)

#endif
