#ifndef CMX_CAMERA_COMPONENT
#define CMX_CAMERA_COMPONENT

// cmx
#include "cmx_component.h"
#include "cmx_register.h"

// lib
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

namespace cmx
{

class CameraComponent : public Component
{
  public:
    CameraComponent();
    ~CameraComponent() = default;

    CLONEABLE(CameraComponent)

    void update(float dt) override;

    void onAttach() override;

    void editor(int i) override;
    void load(tinyxml2::XMLElement *componentElement) override;
    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentComponent) const override;

    const std::shared_ptr<class Camera> getCamera()
    {
        return _camera;
    }

  protected:
    std::shared_ptr<class Camera> _camera;
    bool _mainCamera{false};
};

} // namespace cmx

REGISTER_COMPONENT(cmx::CameraComponent);

#endif
