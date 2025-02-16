#ifndef CMX_MATH
#define CMX_MATH

// lib
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_float3.hpp>

// std
#include <cstdlib>

namespace cmx
{

inline float snapTo(float value)
{
    return value;
}

inline float snapTo(float value, float first)
{
    return first;
}

template <typename... Args> float snapTo(float value, float first, Args... options)
{
    float closest = snapTo(value, options...);
    return (std::abs(value - first) < std::abs(value - closest)) ? first : closest;
}

template <typename T> T lerp(T a, T b, float amount)
{
    return a + (b - a) * amount;
}

template <typename T> T map(T a, T b, T lo, T hi, float amount)
{
    float lerpVal = glm::clamp((amount - lo) / (hi - lo), 0.f, 1.f);

    return lerp(a, b, lerpVal);
}

struct Polygon
{
    glm::vec3 a;
    glm::vec3 b;
    glm::vec3 c;
};

inline glm::vec3 getFaceNormal(const Polygon &polygon)
{
    const glm::vec3 u = polygon.b - polygon.a;
    const glm::vec3 v = polygon.c - polygon.a;

    return glm::cross(u, v);
}

inline glm::vec3 project(const glm::vec3 &a, const glm::vec3 &b)
{
    return (glm::dot(a, b) / glm::dot(b, b)) * b;
}

} // namespace cmx

#endif
