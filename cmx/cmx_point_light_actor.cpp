#include "cmx_point_light_actor.h"

// cmx
#include "cmx_point_light_component.h"
#include "imgui.h"

// lib
#include <glm/ext/scalar_constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace cmx
{

void PointLightActor::onBegin()
{
    Actor::onBegin();

    _pointLightComponent = std::make_shared<PointLightComponent>();
    attachComponent(_pointLightComponent);
}

void PointLightActor::update(float dt)
{
}

} // namespace cmx
