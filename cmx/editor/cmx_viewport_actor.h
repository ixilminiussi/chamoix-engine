#ifndef NDEBUG
#ifndef CMX_VIEWPORT_ACTOR
#define CMX_VIEWPORT_ACTOR

// cmx
#include "cmx_input_manager.h"
#include "cmx_transform.h"

// std
#include <memory>

namespace cmx
{

class ViewportActor
{
  public:
    ViewportActor();

    void update(float dt);

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &, tinyxml2::XMLElement *);
    void load(tinyxml2::XMLElement *);
    void editor();

    void onMovementInput(float dt, glm::vec2);
    void onMouseMovement(float dt, glm::vec2);
    void select(float dt, int val);

    void updateMoveSpeed(float dt, glm::vec2);

    void lock()
    {
        _locked = true;
        _selected = false;
    };
    void unlock()
    {
        _locked = false;
        _selected = false;
        InputManager::setMouseCapture(false, true);
    };

    bool isSelected() const
    {
        return _selected;
    }

    std::shared_ptr<class Camera> getCamera()
    {
        return _camera;
    }

    float _moveSpeed{4.5f};
    float _mouseSensitivity{0.1f};

  private:
    std::shared_ptr<class Camera> _camera;
    bool _selected{false};

    Transform _transform{};

    bool _locked{false};

    glm::vec3 _viewVector{_transform.forward()};
};

} // namespace cmx

#endif
#endif
