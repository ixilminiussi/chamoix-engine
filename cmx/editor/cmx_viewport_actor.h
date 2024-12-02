#pragma once

#include "cmx_actor.h"
#include "cmx_camera_component.h"

// std
#include <memory>

namespace cmx
{

class ViewportActor : public cmx::Actor
{
  public:
    using Actor::Actor;

    void onBegin() override;
    void update(float dt) override;

    void onMovementInput(float dt, glm::vec2);
    void onJumpInput(float dt);
    void onMouseMovement(float dt, glm::vec2);
    void select(float dt, int val);
    void deselect(float dt);

    std::weak_ptr<cmx::CameraComponent> getCamera()
    {
        return camera;
    }

    float moveSpeed{4.5f};
    float mouseSensitivity{0.5f};

  private:
    class std::shared_ptr<cmx::CameraComponent> camera;
    bool selected{false};
};

} // namespace cmx
