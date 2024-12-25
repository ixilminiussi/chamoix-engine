#ifndef CMX_CAMERA_COMPONENT
#define CMX_CAMERA_COMPONENT

#include "cmx_component.h"
#include <glm/trigonometric.hpp>

// lib
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace cmx
{

class CameraComponent : public Component
{
  public:
    CameraComponent() = default;
    ~CameraComponent() = default;

    void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
    void setPerspectiveProjection(float fov, float aspect, float near, float far);

    void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
    void setViewTarget(glm::vec3 target, glm::vec3 direction, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
    void setViewYXZ(glm::vec3 position, glm::vec3 rotation);

    void updateAspectRatio(float aspectRatio);

    void update(float dt) override;
    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &, tinyxml2::XMLElement *) override;

    // getters and setters :: begin
    const glm::mat4 &getProjection() const
    {
        return _projectionMatrix;
    }

    const glm::mat4 &getView() const
    {
        return _viewMatrix;
    }
    // getters and setters :: end

  private:
    glm::mat4 _projectionMatrix{1.f};
    glm::mat4 _viewMatrix{1.f};

    float _screenAspectRatio{};
    float _FOV{80.0f};
    float _nearPlane{.1f};
    float _farPlane{1000.f};
};

} // namespace cmx

#endif
