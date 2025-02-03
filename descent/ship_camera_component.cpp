#include "ship_camera_component.h"
#include "cmx/cmx_math.h"

// cmx
#include <cmath>
#include <cmx/cmx_actor.h>
#include <cmx/cmx_camera.h>
#include <glm/ext/quaternion_trigonometric.hpp>

void ShipCameraComponent::update(float dt)
{
    _cummulatedTime += dt;

    cmx::Transform transform = getWorldSpaceTransform();
    _camera->setViewDirection(transform.position, transform.forward(), transform.up());

    bob();
}

void ShipCameraComponent::bob()
{
    _transform.position.y = std::sin(_cummulatedTime * _bobbingSpeed) * _bobbingRange;
}

void ShipCameraComponent::setTilt(float force)
{
    float angle = glm::radians(cmx::lerp(0.f, _maxTilt, force));
    _transform.rotation = glm::angleAxis(angle, _transform.forward());
}
