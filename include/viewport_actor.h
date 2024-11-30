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

    void onMovementInput(float dt, glm::vec2);
    void onJumpInput(float dt);
    void onMouseMovement(float dt, glm::vec2);
    void select(float dt);
    void deselect(float dt);

    std::weak_ptr<cmx::CameraComponent> getCamera()
    {
        return camera;
    }

  private:
    class std::shared_ptr<cmx::CameraComponent> camera;
    float moveSpeed = 2.f;
    bool selected{false};
};
