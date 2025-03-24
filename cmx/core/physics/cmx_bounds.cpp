#include "cmx_bounds.h"
#include <glm/geometric.hpp>

namespace cmx
{

Bounds::Bounds() : _min{0.f}, _max{0.f}
{
}

Bounds::Bounds(const glm::vec3 startingPoint) : _min{startingPoint}, _max{startingPoint}
{
}

std::array<glm::vec3, 8> Bounds::getAllPoints() const
{
    std::array<glm::vec3, 8> points;
    points[0] = _min;
    points[4] = _max;
    points[1] = glm::vec3{_min.x, _min.y, _max.z};
    points[2] = glm::vec3{_min.x, _max.y, _min.z};
    points[3] = glm::vec3{_min.x, _max.y, _max.z};
    points[5] = glm::vec3{_max.x, _min.y, _min.z};
    points[6] = glm::vec3{_max.x, _min.y, _max.z};
    points[7] = glm::vec3{_max.x, _max.y, _min.z};

    return points;
}

void Bounds::expand(const glm::vec3 &point)
{
    _min.x = std::min(_min.x, point.x);
    _min.y = std::min(_min.y, point.y);
    _min.z = std::min(_min.z, point.z);

    _max.x = std::max(_max.x, point.x);
    _max.y = std::max(_max.y, point.y);
    _max.z = std::max(_max.z, point.z);
}

void Bounds::expand(const std::vector<glm::vec3> &points)
{
    for (const glm::vec3 point : points)
    {
        expand(point);
    }
}

bool Bounds::contains(const glm::vec3 &point) const
{
    return (point.x <= _max.x && point.x >= _min.x && point.y <= _max.y && point.y >= _min.y && point.z <= _max.z &&
            point.z >= _max.z);
}

float Bounds::getLengthX()
{
    return glm::length(_max.x - _min.x);
}

float Bounds::getLengthY()
{
    return glm::length(_max.y - _min.y);
}

float Bounds::getLengthZ()
{
    return glm::length(_max.z - _min.z);
}

const glm::vec3 &Bounds::getMax()
{
    return _max;
}

const glm::vec3 &Bounds::getMin()
{
    return _min;
}

} // namespace cmx
