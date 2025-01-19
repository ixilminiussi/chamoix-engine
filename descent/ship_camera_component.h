#ifndef SHIP_CAMERA_COMPONENT
#define SHIP_CAMERA_COMPONENT

// cmx
#include <cmx/cmx_camera_component.h>

class ShipCameraComponent : public cmx::CameraComponent
{
  public:
    using cmx::CameraComponent::CameraComponent;

    void update(float dt) override;

    void setTilt(float force);

  protected:
    void bob();

    float _maxTilt = 20.f;
    float _bobbingRange = .1;
    float _bobbingSpeed = 4.f;

    float _cummulatedTime{0.f};
};

#endif
