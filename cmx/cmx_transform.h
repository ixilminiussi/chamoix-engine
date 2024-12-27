#ifndef CMX_TRANSFORM
#define CMX_TRANSFORM

// lib
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace cmx
{

struct Transform
{
    glm::vec3 position{};
    glm::vec3 scale{1.f, 1.f, 1.f};
    glm::quat rotation{0.f, 0.f, 0.f, 0.f};

    glm::mat4 mat4();
    glm::mat3 normalMatrix();
    glm::vec3 forward();
    glm::vec3 right();
    glm::vec3 up();
};

Transform operator+(const Transform &a, const Transform &b);

} // namespace cmx

#endif
