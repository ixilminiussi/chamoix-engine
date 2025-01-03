#include "cmx_transform.h"
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace cmx
{

glm::mat4 Transform::mat4()
{
    glm::mat4 rotationMatrix = glm::toMat4(rotation); // Convert quaternion directly to mat4
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);

    return translationMatrix * rotationMatrix * scaleMatrix; // Combine transformations
}

glm::mat3 Transform::normalMatrix()
{
    glm::mat3 rotationMatrix = glm::mat3_cast(rotation);                          // Convert quaternion to mat3
    glm::mat3 scaleMatrix = glm::mat3(glm::scale(glm::mat4(1.0f), 1.0f / scale)); // Inverse scale

    return glm::transpose(glm::inverse(rotationMatrix * scaleMatrix)); // Normal matrix calculation
}

glm::vec3 Transform::forward()
{
    return glm::normalize(rotation * glm::vec3(0.0f, 0.0f, -1.0f)); // Default forward in OpenGL is -Z
}

glm::vec3 Transform::up()
{
    return glm::normalize(rotation * glm::vec3(0.0f, -1.0f, 0.0f)); // Default up is +Y
}

glm::vec3 Transform::right()
{
    return glm::normalize(rotation * glm::vec3(1.0f, 0.0f, 0.0f)); // Default right is +X
}

Transform operator+(const Transform &a, const Transform &b)
{
    Transform c{};
    c.position = a.position + a.rotation * b.position;
    c.rotation = a.rotation + b.rotation;
    c.scale = a.scale + b.scale;

    return c;
}

} // namespace cmx
