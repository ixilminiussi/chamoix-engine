#ifndef CMX_MATH
#define CMX_MATH

// lib
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/scalar_constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext/vector_float3.hpp>
#include <glm/gtx/norm.hpp>

// std
#include <array>
#include <cstdlib>
#include <vector>

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

inline glm::vec3 getFaceNormal(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c)
{
    const glm::vec3 u = b - a;
    const glm::vec3 v = c - a;

    return glm::cross(u, v);
}

inline float getTripleProduct(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c, const glm::vec3 &d)
{
    const glm::vec3 u = b - a;
    const glm::vec3 v = c - a;
    const glm::vec3 w = d - a;

    return glm::dot(glm::cross(u, v), w);
}

inline glm::vec3 project(const glm::vec3 &a, const glm::vec3 &b)
{
    return (glm::dot(a, b) / glm::dot(b, b)) * b;
}

inline float getFaceVolume(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c)
{
    return glm::length(glm::cross(b - a, c - a)) * glm::length(c - a) / 2.0f;
}

inline glm::vec3 getFaceCenter(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c)
{
    return (a + b + c) / 3.0f;
}

inline float signedDistanceToFace(const glm::vec3 &p, const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c)
{
    return glm::dot(getFaceNormal(a, b, c), p - a);
}

inline float cofactor(const glm::mat4 &M, int i_, int j_)
{
    glm::mat3 minor;
    int minorRow = 0;
    for (int i = 0; i < 3; ++i)
    {
        if (i == i_)
            continue;
        int minorCol = 0;
        for (int j = 0; j < 3; ++j)
        {
            if (j == j_)
                continue;
            minor[minorRow][minorCol] = M[i][j];
            minorCol++;
        }
        minorRow++;
    }

    return glm::determinant(minor);
}

inline int compareSigns(float a, float b)
{
    if (a > 0.0f && b > 0.0f)
    {
        return 1;
    }
    if (a < 0.0f && b < 0.0f)
    {
        return 1;
    }
    return 0;
}

inline void getOrtho(const glm::vec3 &a, glm::vec3 &u, glm::vec3 &v)
{
    u = glm::normalize(glm::cross(a, glm::vec3(1, 0, 0)));
    if (glm::length2(u) < glm::epsilon<float>())
    {
        u = glm::normalize(glm::cross(a, glm::vec3(0, 1, 0)));
    }
    v = glm::cross(a, u);
}

} // namespace cmx

#endif
