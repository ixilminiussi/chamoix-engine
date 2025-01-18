#include "ship_camera_component.h"

// cmx
#include <cmx/cmx_actor.h>
#include <cmx/cmx_camera.h>

void ShipCameraComponent::update(float dt)
{
    _camera->setViewDirection(getParent()->transform.position, getParent()->transform.forward(),
                              getParent()->transform.up());
}
