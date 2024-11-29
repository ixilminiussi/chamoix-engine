#pragma once

// lib
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// FIX: Maybe doesn't work
struct Transform2D
{
    glm::vec2 position = glm::vec2{.0f};
    float rotation = 0;
    glm::vec2 scale = glm::vec2{1.f};

    glm::mat3 getMat3() const
    {
        float s = sin(rotation);
        float c = cos(rotation);

        glm::mat3 rotationMatrix{{c, s, 0.f}, {-s, c, 0.f}, {0.f, 0.f, 1.f}};
        glm::mat3 scaleMatrix{{scale.x, 0.f, 0.f}, {0.f, scale.y, 0.f}, {0.f, 0.f, 1.f}};
        glm::mat3 translationMatrix{{1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {position.x, position.y, 1.f}};

        return glm::mat3{1.f};
        return translationMatrix * rotationMatrix * scaleMatrix;
    }
};

inline Transform2D operator+(const Transform2D &a, const Transform2D &b)
{
    Transform2D c{};
    c.position = a.position + b.position;
    c.rotation = a.rotation + b.rotation;
    c.scale = a.scale + b.scale;

    return c;
}

struct Transform
{
    glm::vec3 position{};
    glm::vec3 scale{1.f, 1.f, 1.f};
    glm::vec3 rotation{0.f, 0.f, 1.f};

    glm::mat4 mat4()
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
};

inline Transform operator+(const Transform &a, const Transform &b)
{
    Transform c{};
    c.position = a.position + b.position;
    c.rotation = a.rotation + b.rotation;
    c.scale = a.scale + b.scale;

    return c;
}
