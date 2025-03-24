#ifndef CMX_MATH
#define CMX_MATH

// lib
#include <glm/ext/quaternion_geometric.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext/vector_float3.hpp>
#include <glm/gtx/norm.hpp>

// std
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

inline int findPointFurthestInDir(const std::vector<glm::vec3> &candidates, const glm::vec3 &dir)
{
    int j = 0;
    float maxDst = 0.f;

    for (int i = 1; i < candidates.size(); i++)
    {
        float newDst = glm::dot(candidates[i], dir);
        if (newDst > maxDst)
        {
            newDst = maxDst;
            j = i;
        }
    }

    return j;
}

inline int findPointFurthestFromLine(const std::vector<glm::vec3> &candidates, const glm::vec3 &a, const glm::vec3 &b)
{
    int j = 0;
    float maxDst = 0.f;

    for (int i = 1; i < candidates.size(); i++)
    {
        const glm::vec3 &candidate = candidates[i];
        if (candidate == a || candidate == b)
        {
            continue;
        }

        float newDst = glm::length2(candidate - a) + glm::length2(candidate - b);

        if (newDst > maxDst)
        {
            newDst = maxDst;
            j = i;
        }
    }

    return j;
}

inline int findPointFurthestFromFace(const std::vector<glm::vec3> &candidates, const glm::vec3 &a, const glm::vec3 &b,
                                     const glm::vec3 &c)
{
    int j = 0;
    float maxDst = 0.f;

    for (int i = 1; i < candidates.size(); i++)
    {
        const glm::vec3 &candidate = candidates[i];

        if (candidate == a || candidate == b || candidate == c)
        {
            continue;
        }

        float newDst = std::abs(getTripleProduct(a, b, c, candidate));

        if (newDst > maxDst)
        {
            newDst = maxDst;
            j = i;
        }
    }

    return j;
}

inline float getFaceVolume(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c)
{
    return glm::length(glm::cross(b - a, c - a)) * glm::length(c - a) / 2.0f;
}

inline glm::vec3 getFaceCenter(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c)
{
    return (a + b + c) / 3.0f;
}

} // namespace cmx

#endif
