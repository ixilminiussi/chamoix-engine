#ifndef SHIP_CAMERA_COMPONENT
#define SHIP_CAMERA_COMPONENT

// cmx
#include <cmx/cmx_camera_component.h>

class ShipCameraComponent : public cmx::CameraComponent
{
  public:
    using cmx::CameraComponent::CameraComponent;

    void update(float dt) override;

  protected:
};

#endif
