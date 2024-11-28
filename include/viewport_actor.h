#pragma once

#include "cmx_actor.h"
#include "cmx_camera_component.h"

// std
#include <memory>

class ViewportActor : public cmx::Actor
{
  public:
    using Actor::Actor;

    void onBegin() override;
    void update(float dt) override;

    std::weak_ptr<cmx::CmxCameraComponent> getCamera()
    {
        return camera;
    }

  private:
    class std::shared_ptr<cmx::CmxCameraComponent> camera;
};
