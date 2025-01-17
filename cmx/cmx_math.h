#ifndef CMX_MATH
#define CMX_MATH

#include <cstdlib>
#include <glm/ext/vector_float3.hpp>
namespace cmx
{

float snapTo(float value)
{
    return value;
}

float snapTo(float value, float first)
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
