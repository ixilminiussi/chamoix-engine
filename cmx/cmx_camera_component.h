#ifndef CMX_CAMERA_COMPONENT
#define CMX_CAMERA_COMPONENT

#include "cmx_component.h"

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

    void update(float dt) override;

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &, tinyxml2::XMLElement *) override;

    const std::shared_ptr<class Camera> getCamera()
    {
        return _camera;
    }

  private:
    std::shared_ptr<class Camera> _camera;
};

} // namespace cmx

#endif
