#include "cmx_shapes.h"
#include "cmx_assets_manager.h"
#include "cmx_edge_render_system.h"
#include "cmx_frame_info.h"
#include "cmx_model.h"
#include "cmx_physics_actor.h"
#include "cmx_physics_component.h"
#include "cmx_primitives.h"

// lib
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/matrix.hpp>
#include <memory>
#include <spdlog/spdlog.h>

namespace cmx
{

CmxShape::CmxShape(Transformable *parent) : _parent{parent}
{
    _overlappingComponents[0] = {};
    _overlappingComponents[1] = {};
}

bool CmxShape::wasOverlapping(PhysicsComponent *component) const
{
    auto search = _overlappingComponents[_alternativeBuffer].find(component);

    return (search != _overlappingComponents[_alternativeBuffer].end());
}

bool CmxShape::isOverlapping(PhysicsComponent *component) const
{
    auto search = _overlappingComponents[_buffer].find(component);

    return (search != _overlappingComponents[_buffer].end());
}

bool CmxShape::isOverlapping() const
{
    return !_overlappingComponents[_buffer].empty();
}

void CmxShape::addOverlappingComponent(PhysicsComponent *component)
{
    _overlappingComponents[_buffer].emplace(component);
}

void CmxShape::reassess()
{
    if (_overlappingComponents[0].empty() && _overlappingComponents[1].empty())
    {
        swapBuffer();
        return;
    }

    auto currentIt = _overlappingComponents[_buffer].begin();
    auto previousIt = _overlappingComponents[_alternativeBuffer].begin();

    while (previousIt != _overlappingComponents[_alternativeBuffer].end())
    {
        if (*currentIt == *previousIt)
        {
            currentIt++;
            previousIt++;
            continue;
        }

        if (!isOverlapping(*previousIt))
        {
            if (auto ourComponent = dynamic_cast<PhysicsComponent *>(_parent))
            {
                spdlog::info("yes");
                if (auto ourActor = dynamic_cast<PhysicsActor *>(ourComponent->getParent()))
                {
                    if (*previousIt != nullptr)
                    {
                        ourActor->onEndOverlap(ourComponent, *previousIt, (*previousIt)->getParent());
                    }
                    else
                    {
                        ourActor->onEndOverlap(ourComponent, nullptr, nullptr);
                    }
                }
            }
        }
        if (currentIt != _overlappingComponents[_buffer].end())
        {
            currentIt++;
        }
        previousIt++;
    }
}

void CmxShape::swapBuffer()
{
    _alternativeBuffer = _buffer;
    _buffer = (_buffer + 1) % 2;
    _overlappingComponents[_buffer].clear();
}

CmxSphere::CmxSphere(const glm::vec3 &center, float radius, Transformable *parent)
    : _relCenter{center, 1.0}, _relRadius{radius}, CmxShape(parent)
{
    _absCenter = _relCenter;
    _absRadius = _relRadius;
}

void CmxSphere::render(const FrameInfo &frameInfo, VkPipelineLayout pipelineLayout,
                       std::shared_ptr<AssetsManager> assetsManager)
{
    EdgePushConstantData push{};

    Transform transform = Transform{_absCenter, glm::vec3{_absRadius}, glm::vec3{0.f}};
    push.modelMatrix = transform.mat4();
    push.color = isOverlapping() ? glm::vec3{1.f, 0.f, 0.f} : glm::vec3{0.f, 1.f, 1.f};

    vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(EdgePushConstantData),
                       &push);

    assetsManager->getModel(PRIMITIVE_SPHERE)->bind(frameInfo.commandBuffer);
    assetsManager->getModel(PRIMITIVE_SPHERE)->draw(frameInfo.commandBuffer);
}

bool CmxSphere::overlapsWith(const CmxShape &other) const
{
    return other.overlapsWith(*this);
}

bool CmxSphere::overlapsWith(const CmxSphere &other) const
{
    return (glm::length(_absCenter - other._absCenter) <= (_absRadius + other._absRadius));
}

bool CmxSphere::overlapsWith(const CmxCuboid &other) const
{
    return other.overlapsWith(*this);
}

void CmxSphere::expandToInclude(const glm::vec3 &other)
{
    const glm::mat4 invMat4 = glm::inverse(_parent->getAbsoluteTransform().mat4());
    glm::vec4 newP = invMat4 * glm::vec4{other, 1.0f};
    newP.w = 1.0f;

    _relRadius = std::min(glm::length(newP - _relCenter), _relRadius);
}

void CmxSphere::updateDimensions()
{
    if (_parent == nullptr)
        return;

    const Transform absTransform = _parent->getAbsoluteTransform();
    _absCenter = absTransform.mat4() * _relCenter;
    _absCenter.w = 1.0f;
    _absRadius = _relRadius * std::max(std::max(absTransform.scale.x, absTransform.scale.y), absTransform.scale.z);
}

glm::vec3 CmxPolygon::getNormal()
{
    return glm::normalize(glm::cross(*b - *a, *c - *a));
}

void CmxCuboid::render(const FrameInfo &frameInfo, VkPipelineLayout pipelineLayout,
                       std::shared_ptr<AssetsManager> assetsManager)
{
    // manually push all the vertices at this point...
}

CmxCuboid::CmxCuboid(const glm::vec3 &cornerA, const glm::vec3 &cornerG, Transformable *parent)
    : _relA{cornerA, 1.0}, _relB{cornerG.x, cornerA.y, cornerA.z, 1.0f}, _relC{cornerG.x, cornerA.y, cornerG.z, 1.0f},
      _relD{cornerA.x, cornerA.y, cornerG.z, 1.0f}, _relE{cornerA.x, cornerG.y, cornerA.z, 1.0f},
      _relF{cornerG.x, cornerG.y, cornerA.z, 1.0f}, _relG{cornerG, 1.0}, _relH{cornerA.x, cornerG.y, cornerG.z, 1.0f},
      CmxShape(parent)
{
    _absA = _relA;
    _absB = _relB;
    _absC = _relC;
    _absD = _relD;
    _absE = _relE;
    _absF = _relF;
    _absG = _relG;
    _absH = _relH;
}

bool CmxCuboid::overlapsWith(const CmxShape &other) const
{
    return other.overlapsWith(*this);
}

bool CmxCuboid::overlapsWith(const CmxCuboid &other) const
{
    const Transform &transform = _parent != nullptr ? _parent->getAbsoluteTransform() : Transform::ONE;
    const glm::mat4 inverseMat4 = glm::inverse(transform.mat4());

    glm::vec4 p = inverseMat4 * other._absA;
    float minX = p.x, minY = p.y, minZ = p.z, maxX = p.x, maxY = p.y, maxZ = p.z;

    auto update = [&minX, &minY, &minZ, &maxX, &maxY, &maxZ](glm::vec4 p) {
        minX = std::min(minX, p.x);
        minY = std::min(minY, p.y);
        minZ = std::min(minZ, p.z);
        maxX = std::max(maxX, p.x);
        maxY = std::max(maxY, p.y);
        maxZ = std::max(maxZ, p.z);
    };

    update(inverseMat4 * other._absB);
    update(inverseMat4 * other._absC);
    update(inverseMat4 * other._absD);
    update(inverseMat4 * other._absE);
    update(inverseMat4 * other._absF);
    update(inverseMat4 * other._absG);
    update(inverseMat4 * other._absH);

    if (maxX < std::min(_absA.x, _absG.x))
        return false;
    if (maxY < std::min(_absA.y, _absG.y))
        return false;
    if (maxZ < std::min(_absA.z, _absG.z))
        return false;
    if (minX > std::max(_absA.x, _absG.x))
        return false;
    if (minY > std::max(_absA.y, _absG.y))
        return false;
    if (minZ > std::max(_absA.z, _absG.z))
        return false;

    return true;
}

bool CmxCuboid::overlapsWith(const CmxSphere &other) const
{
    const Transform &transform = _parent != nullptr ? _parent->getAbsoluteTransform() : Transform::ONE;

    glm::mat4 scaler = glm::scale(glm::mat4(1.0f), transform.scale);

    glm::vec4 scaledA = scaler * _relA;
    glm::vec4 scaledG = scaler * _relG;

    glm::vec4 newP = glm::inverse(transform.mat4()) * other._absCenter;

    if (newP.x + other._absRadius < std::min(scaledA.x, scaledG.x))
        return false;
    if (newP.x - other._absRadius > std::max(scaledA.x, scaledG.x))
        return false;

    if (newP.y + other._absRadius < std::min(scaledA.y, scaledG.y))
        return false;
    if (newP.y - other._absRadius > std::max(scaledA.y, scaledG.y))
        return false;

    if (newP.z + other._absRadius < std::min(scaledA.z, scaledG.z))
        return false;
    if (newP.z - other._absRadius > std::max(scaledA.z, scaledG.z))
        return false;

    return true;
}

void CmxCuboid::expandToInclude(const glm::vec3 &)
{
}

void CmxCuboid::updateDimensions()
{
    if (_parent == nullptr)
        return;

    const glm::mat4 mat4 = _parent->getAbsoluteTransform().mat4();

    _absA = mat4 * _relA;
    _absB = mat4 * _relB;
    _absC = mat4 * _relC;
    _absD = mat4 * _relD;
    _absE = mat4 * _relE;
    _absF = mat4 * _relF;
    _absG = mat4 * _relG;
    _absH = mat4 * _relH;
}

} // namespace cmx
