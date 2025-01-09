#include "cmx_shapes.h"

// lib
#include <glm/ext/vector_float3.hpp>
#include <glm/matrix.hpp>

namespace cmx
{

CmxSphere::CmxSphere(const glm::vec3 &center, float radius, const Transformable *parent)
    : _relCenter{center, 1.0}, _relRadius{radius}, CmxShape(parent)
{
    _center = _relCenter;
    _radius = _relRadius;
}

bool CmxSphere::overlapsWith(const CmxSphere &)
{
}

void CmxSphere::expandToInclude(const glm::vec3 &p)
{
    glm::mat4 invMat4 = glm::inverse(_parent->getAbsoluteTransform().mat4());
    glm::vec4 newP = invMat4 * glm::vec4{p, 1.0f};

    _relRadius = std::min(glm::length(newP - _relCenter), _relRadius);
}

void CmxSphere::updateDimensions()
{
    if (_parent == nullptr)
        return;

    const Transform absTransform = _parent->getAbsoluteTransform();
    _center = absTransform.mat4() * _relCenter;
    _radius = _relRadius * std::max(std::max(absTransform.scale.x, absTransform.scale.y), absTransform.scale.z);
}

glm::vec3 CmxPolygon::getNormal()
{
    return glm::normalize(glm::cross(*b - *a, *c - *a));
}

CmxCuboid::CmxCuboid(const glm::vec3 &cornerA, const glm::vec3 &cornerG, const Transformable *parent)
    : _relA{cornerA, 1.0}, _relB{cornerG.x, cornerA.y, cornerA.z, 1.0f}, _relC{cornerG.x, cornerA.y, cornerG.z, 1.0f},
      _relD{cornerA.x, cornerA.y, cornerG.z, 1.0f}, _relE{cornerA.x, cornerG.y, cornerA.z, 1.0f},
      _relF{cornerG.x, cornerG.y, cornerA.z, 1.0f}, _relG{cornerG, 1.0}, _relH{cornerA.x, cornerG.y, cornerG.z, 1.0f},
      CmxShape(parent)
{
    _a = _relA;
    _b = _relB;
    _c = _relC;
    _d = _relD;
    _e = _relE;
    _f = _relF;
    _g = _relG;
    _h = _relH;
}

bool CmxCuboid::overlapsWith(const CmxCuboid &)
{
}

bool CmxCuboid::overlapsWith(const CmxSphere &)
{
}

void CmxCuboid::expandToInclude(const glm::vec3 &)
{
}

void CmxCuboid::updateDimensions()
{
    if (_parent == nullptr)
        return;

    glm::mat4 mat4 = _parent->getAbsoluteTransform().mat4();

    _a = mat4 * _relA;
    _b = mat4 * _relB;
    _c = mat4 * _relC;
    _d = mat4 * _relD;
    _e = mat4 * _relE;
    _f = mat4 * _relF;
    _g = mat4 * _relG;
    _h = mat4 * _relH;
}

} // namespace cmx
