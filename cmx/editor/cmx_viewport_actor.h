#ifndef CMX_VIEWPORT_ACTOR
#define CMX_VIEWPORT_ACTOR

#include "cmx_actor.h"

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

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &, tinyxml2::XMLElement *) override;
    void load(tinyxml2::XMLElement *) override;

    void onMovementInput(float dt, glm::vec2);
    void onJumpInput(float dt);
    void onMouseMovement(float dt, glm::vec2);
    void select(float dt, int val);
    void deselect(float dt);

    std::weak_ptr<class cmx::CameraComponent> getCamera()
    {
        return _camera;
    }

    float _moveSpeed{4.5f};
    float _mouseSensitivity{0.5f};

  private:
    std::shared_ptr<class cmx::CameraComponent> _camera;
    bool _selected{false};
    glm::vec3 _viewVector{transform.forward()};
};

} // namespace cmx

#endif
