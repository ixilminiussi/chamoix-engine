#include "cmx_camera.h"
#include "imgui.h"

namespace cmx
{

void Camera::setOrthographicProjection(float left, float right, float top, float bottom, float near, float far)
{
    _projectionMatrix = glm::ortho(left, right, bottom, top, near, far);
}

void Camera::setPerspectiveProjection(float fov, float aspect, float near, float far)
{
    assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
    const float tanHalfFovy = tan(glm::radians(fov) / 2.f);
    _projectionMatrix = glm::mat4{0.0f};
    _projectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
    _projectionMatrix[1][1] = 1.f / (tanHalfFovy);
    _projectionMatrix[2][2] = far / (far - near);
    _projectionMatrix[2][3] = 1.f;
    _projectionMatrix[3][2] = -(far * near) / (far - near);
}

void Camera::setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up)
{
    const glm::vec3 w{glm::normalize(direction)};
    const glm::vec3 u{glm::normalize(glm::cross(w, up))};
    const glm::vec3 v{glm::cross(w, u)};

    _viewMatrix = glm::mat4{1.f};
    _viewMatrix[0][0] = u.x;
    _viewMatrix[1][0] = u.y;
    _viewMatrix[2][0] = u.z;
    _viewMatrix[0][1] = v.x;
    _viewMatrix[1][1] = v.y;
    _viewMatrix[2][1] = v.z;
    _viewMatrix[0][2] = w.x;
    _viewMatrix[1][2] = w.y;
    _viewMatrix[2][2] = w.z;
    _viewMatrix[3][0] = -glm::dot(u, position);
    _viewMatrix[3][1] = -glm::dot(v, position);
    _viewMatrix[3][2] = -glm::dot(w, position);
}

void Camera::setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up)
{
    setViewDirection(position, target - position, up);
}

void Camera::setViewYXZ(glm::vec3 position, glm::vec3 rotation)
{
    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);
    const glm::vec3 u{(c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1)};
    const glm::vec3 v{(c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3)};
    const glm::vec3 w{(c2 * s1), (-s2), (c1 * c2)};
    _viewMatrix = glm::mat4{1.f};
    _viewMatrix[0][0] = u.x;
    _viewMatrix[1][0] = u.y;
    _viewMatrix[2][0] = u.z;
    _viewMatrix[0][1] = v.x;
    _viewMatrix[1][1] = v.y;
    _viewMatrix[2][1] = v.z;
    _viewMatrix[0][2] = w.x;
    _viewMatrix[1][2] = w.y;
    _viewMatrix[2][2] = w.z;
    _viewMatrix[3][0] = -glm::dot(u, position);
    _viewMatrix[3][1] = -glm::dot(v, position);
    _viewMatrix[3][2] = -glm::dot(w, position);
}

void Camera::updateAspectRatio(float aspectRatio)
{
    if (aspectRatio != _screenAspectRatio)
    {
        _screenAspectRatio = aspectRatio;
    }

    float boxSize = 20.f;
    // setOrthographicProjection(-aspectRatio * boxSize, aspectRatio * boxSize, boxSize, -boxSize, -boxSize, boxSize);
    setPerspectiveProjection(_FOV, _screenAspectRatio, _nearPlane, _farPlane);
}

void Camera::editor()
{
    ImGui::SliderFloat("FOV", &_FOV, 10.f, 160.f);
    ImGui::SliderFloat("far plane", &_farPlane, 1.f, 100000.f);
    ImGui::SliderFloat("near plane", &_nearPlane, 1.f, 100000.f);
}

} // namespace cmx
