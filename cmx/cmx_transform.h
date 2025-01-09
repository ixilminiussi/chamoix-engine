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

    glm::mat4 mat4() const;
    glm::mat3 normalMatrix() const;
    glm::vec3 forward() const;
    glm::vec3 right() const;
    glm::vec3 up() const;

    static Transform ONE;
};

class Transformable
{
  public:
    virtual const Transform &getRelativeTransform() const = 0;
    virtual Transform getAbsoluteTransform() const = 0;
};

Transform operator+(const Transform &a, const Transform &b);

} // namespace cmx

#endif
