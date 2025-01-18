#ifndef CMX_MATH
#define CMX_MATH

// lib
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

} // namespace cmx

#endif
