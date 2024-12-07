#pragma once

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
        return projectionMatrix;
    }

    const glm::mat4 &getView() const
    {
        return viewMatrix;
    }
    // getters and setters :: end

  private:
    glm::mat4 projectionMatrix{1.f};
    glm::mat4 viewMatrix{1.f};

    float screenAspectRatio{};
    float FOV{80.0f};
    float nearPlane{.1f};
    float farPlane{1000.f};
};

} // namespace cmx
