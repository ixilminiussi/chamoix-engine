#ifndef CMX_CAMERA
#define CMX_CAMERA

// lib
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace cmx
{

class Camera
{
  public:
    void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
    void setPerspectiveProjection(float fov, float aspect, float near, float far);

    void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
    void setViewTarget(glm::vec3 target, glm::vec3 direction, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
    void setViewYXZ(glm::vec3 position, glm::vec3 rotation);

    void updateAspectRatio(float aspectRatio);

    // getters and setters :: begin
    const glm::mat4 &getProjection() const
    {
        return _projectionMatrix;
    }

    const glm::mat4 &getView() const
    {
        return _viewMatrix;
    }

    void editor();

    float getFOV() const
    {
        return _FOV;
    }
    void setFOV(float FOV)
    {
        _FOV = FOV;
    }
    float getNearPlane()
    {
        return _nearPlane;
    }
    void setNearPlane(float nearPlane)
    {
        _nearPlane = nearPlane;
    }
    float getFarPlane()
    {
        return _farPlane;
    }
    void setFarPlane(float farPlane)
    {
        _farPlane = farPlane;
    }
    glm::vec3 getPosition() const
    {
        return glm::vec3(glm::inverse(_viewMatrix)[3]);
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
