#include "cmx_transform.h"

namespace cmx
{

glm::mat4 Transform::mat4()
{
    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);
    return glm::mat4{{
                         scale.x * (c1 * c3 + s1 * s2 * s3),
                         scale.x * (c2 * s3),
                         scale.x * (c1 * s2 * s3 - c3 * s1),
                         0.0f,
                     },
                     {
                         scale.y * (c3 * s1 * s2 - c1 * s3),
                         scale.y * (c2 * c3),
                         scale.y * (c1 * c3 * s2 + s1 * s3),
                         0.0f,
                     },
                     {
                         scale.z * (c2 * s1),
                         scale.z * (-s2),
                         scale.z * (c1 * c2),
                         0.0f,
                     },
                     {position.x, position.y, position.z, 1.0f}};
}

glm::mat3 Transform::normalMatrix()
{
    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);
    const glm::vec3 invScale = 1.0f / scale;
    return glm::mat3{
        {
            invScale.x * (c1 * c3 + s1 * s2 * s3),
            invScale.x * (c2 * s3),
            invScale.x * (c1 * s2 * s3 - c3 * s1),
        },
        {
            invScale.y * (c3 * s1 * s2 - c1 * s3),
            invScale.y * (c2 * c3),
            invScale.y * (c1 * c3 * s2 + s1 * s3),
        },
        {
            invScale.z * (c2 * s1),
            invScale.z * (-s2),
            invScale.z * (c1 * c2),
        },
    };
}

glm::vec3 Transform::forward()
{
    return (glm::vec3{glm::sin(rotation.y) * glm::cos(rotation.x), glm::sin(rotation.x),
                      glm::cos(rotation.y) * glm::cos(rotation.x)});
}

glm::vec3 Transform::right()
{
    return glm::normalize(glm::vec3{glm::cos(rotation.y), 0.0f, -glm::sin(rotation.y)});
}

glm::vec3 Transform::up()
{
    return glm::normalize(glm::cross(right(), forward()));
}

Transform operator+(const Transform &a, const Transform &b)
{
    Transform c{};
    c.position = a.position + b.position;
    c.rotation = a.rotation + b.rotation;
    c.scale = a.scale + b.scale;

    return c;
}

} // namespace cmx
