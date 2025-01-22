#include "cmx_shapes.h"
#include "cmx_assets_manager.h"
#include "cmx_edge_render_system.h"
#include "cmx_frame_info.h"
#include "cmx_model.h"
#include "cmx_physics_actor.h"
#include "cmx_physics_component.h"
#include "cmx_primitives.h"
#include "imgui.h"

// lib
#include <glm/ext/scalar_constants.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/matrix.hpp>
#include <spdlog/spdlog.h>

namespace cmx
{

CmxShape::CmxShape(Transformable *parent) : _parent{parent}
{
    _overlappingComponents[0] = {};
    _overlappingComponents[1] = {};
}

Transform CmxShape::getAbsoluteTransform() const
{
    if (_parent)
    {
        return _parent->getAbsoluteTransform();
    }
    return Transform::ONE;
}

const Transform &CmxShape::getRelativeTransform() const
{
    return Transform::ONE;
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

CmxSphere::CmxSphere(Transformable *parent) : CmxShape{parent}
{
}

std::string CmxSphere::getName() const
{
    return PRIMITIVE_SPHERE;
}

void CmxSphere::render(const FrameInfo &frameInfo, VkPipelineLayout pipelineLayout, AssetsManager *assetsManager)
{
    EdgePushConstantData push{};

    Transform transform = getAbsoluteTransform();

    push.modelMatrix = transform.mat4();
    push.color = isOverlapping() ? glm::vec3{1.f, 0.f, 0.f} : glm::vec3{0.f, 1.f, 1.f};

    vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(EdgePushConstantData),
                       &push);

    if (CmxModel *model = assetsManager->getModel(PRIMITIVE_SPHERE))
    {
        model->bind(frameInfo.commandBuffer);
        model->draw(frameInfo.commandBuffer);
    }
}

bool CmxSphere::overlapsWith(const CmxShape &other, HitInfo &hitInfo) const
{
    if (!(mask & other.mask))
        return false;

    bool b = other.overlapsWith(*this, hitInfo);

    if (b)
    {
        hitInfo.flip();
        return true;
    }

    return false;
}

bool CmxSphere::overlapsWith(const CmxSphere &other, HitInfo &hitInfo) const
{
    if (!(mask & other.mask))
        return false;

    hitInfo.normal = (other.getCenter() - getCenter());
    float minDist = getRadius() + other.getRadius();
    hitInfo.depth = minDist - glm::length(hitInfo.normal);
    hitInfo.normal = (hitInfo.depth <= glm::epsilon<float>()) ? hitInfo.normal : glm::normalize(hitInfo.normal);

    return hitInfo.depth > 0.f;
}

bool CmxSphere::overlapsWith(const CmxPlane &other, HitInfo &hitInfo) const
{
    if (!(mask & other.mask))
        return false;

    bool b = other.overlapsWith(*this, hitInfo);

    if (b)
    {
        hitInfo.flip();
        return true;
    }

    return false;
}

bool CmxSphere::overlapsWith(const CmxCuboid &other, HitInfo &hitInfo) const
{
    if (!(mask & other.mask))
        return false;

    bool b = other.overlapsWith(*this, hitInfo);

    if (b)
    {
        hitInfo.flip();
        return true;
    }

    return false;
}

glm::vec3 CmxSphere::getCenter() const
{
    return getAbsoluteTransform().position;
}

float CmxSphere::getRadius() const
{
    Transform transform = getAbsoluteTransform();

    return std::max(std::max(transform.scale.x, transform.scale.y), transform.scale.z);
}

CmxPlane::CmxPlane(Transformable *parent) : CmxShape{parent}
{
}

std::string CmxPlane::getName() const
{
    return PRIMITIVE_PLANE;
}

void CmxPlane::render(const FrameInfo &frameInfo, VkPipelineLayout pipelineLayout, AssetsManager *assetsManager)
{
    EdgePushConstantData push{};

    Transform transform = getAbsoluteTransform();

    push.modelMatrix = transform.mat4();
    push.color = isOverlapping() ? glm::vec3{1.f, 0.f, 0.f} : glm::vec3{0.f, 1.f, 1.f};

    vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(EdgePushConstantData),
                       &push);

    if (CmxModel *model = assetsManager->getModel(PRIMITIVE_PLANE))
    {
        model->bind(frameInfo.commandBuffer);
        model->draw(frameInfo.commandBuffer);
    }
}

bool CmxPlane::overlapsWith(const CmxShape &other, HitInfo &hitInfo) const
{
    if (!(mask & other.mask))
        return false;

    bool b = other.overlapsWith(*this, hitInfo);

    if (b)
    {
        hitInfo.flip();
        return true;
    }

    return false;
}

bool CmxPlane::overlapsWith(const CmxPlane &other, HitInfo &hitInfo) const
{
    if (!(mask & other.mask))
        return false;

    return false;
}

bool CmxPlane::overlapsWith(const CmxCuboid &other, HitInfo &hitInfo) const
{
    if (!(mask & other.mask))
        return false;

    return false;
}

bool CmxPlane::overlapsWith(const CmxSphere &other, HitInfo &hitInfo) const
{
    if (!(mask & other.mask))
        return false;

    Transform transform = getAbsoluteTransform();

    glm::mat4 scaler = glm::scale(glm::mat4(1.0f), transform.scale);

    glm::vec4 scaledMin = getMin(scaler);
    glm::vec4 scaledMax = getMax(scaler);

    glm::mat4 noScale = transform.mat4_noScale();
    glm::vec4 newCenter = glm::inverse(noScale) * glm::vec4{other.getCenter(), 1.0f};

    // OBB
    // get closest point
    glm::vec4 closestPoint{1.f};
    closestPoint.x = (newCenter.x < scaledMin.x)   ? scaledMin.x
                     : (newCenter.x > scaledMax.x) ? scaledMax.x
                                                   : newCenter.x;
    closestPoint.z = (newCenter.z < scaledMin.z)   ? scaledMin.z
                     : (newCenter.z > scaledMax.z) ? scaledMax.z
                                                   : newCenter.z;
    closestPoint.y = scaledMin.y;

    hitInfo.normal = glm::vec3(noScale * (newCenter - closestPoint));
    hitInfo.depth = other.getRadius() - glm::length(hitInfo.normal);
    hitInfo.normal = (hitInfo.depth <= glm::epsilon<float>()) ? hitInfo.normal : glm::normalize(hitInfo.normal);

    return hitInfo.depth > 0;
}

glm::vec4 CmxPlane::getMin(const glm::mat4 &mat4) const
{
    return mat4 * glm::vec4{-1.f, 0.f, -1.f, 1.f};
}

glm::vec4 CmxPlane::getMax(const glm::mat4 &mat4) const
{
    return mat4 * glm::vec4{1.f, 0.f, 1.f, 1.f};
}

CmxCuboid::CmxCuboid(Transformable *parent) : CmxShape{parent}
{
}

std::string CmxCuboid::getName() const
{
    return PRIMITIVE_CUBE;
}

void CmxCuboid::render(const FrameInfo &frameInfo, VkPipelineLayout pipelineLayout, AssetsManager *assetsManager)
{
    EdgePushConstantData push{};

    Transform transform = getAbsoluteTransform();

    push.modelMatrix = transform.mat4();
    push.color = isOverlapping() ? glm::vec3{1.f, 0.f, 0.f} : glm::vec3{0.f, 1.f, 1.f};

    vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(EdgePushConstantData),
                       &push);

    if (CmxModel *model = assetsManager->getModel(PRIMITIVE_CUBE))
    {
        model->bind(frameInfo.commandBuffer);
        model->draw(frameInfo.commandBuffer);
    }
}

bool CmxCuboid::overlapsWith(const CmxShape &other, HitInfo &hitInfo) const
{
    if (!(mask & other.mask))
        return false;

    bool b = other.overlapsWith(*this, hitInfo);

    if (b)
    {
        hitInfo.flip();
        return true;
    }

    return false;
}

bool CmxCuboid::overlapsWith(const CmxPlane &other, HitInfo &hitInfo) const
{
    if (!(mask & other.mask))
        return false;

    return false;
}

bool CmxCuboid::overlapsWith(const CmxCuboid &other, HitInfo &hitInfo) const
{
    if (!(mask & other.mask))
        return false;

    return false;
}

bool CmxCuboid::overlapsWith(const CmxSphere &other, HitInfo &hitInfo) const
{
    Transform transform = getAbsoluteTransform();

    glm::mat4 scaler = glm::scale(glm::mat4(1.0f), transform.scale);

    glm::vec4 scaledMin = getMin(scaler);
    glm::vec4 scaledMax = getMax(scaler);

    glm::mat4 noScale = transform.mat4_noScale();
    glm::vec4 newCenter = glm::inverse(noScale) * glm::vec4{other.getCenter(), 1.0f};

    // OBB
    // get closest point
    glm::vec4 closestPoint{1.f};
    closestPoint.x = (newCenter.x < scaledMin.x)   ? scaledMin.x
                     : (newCenter.x > scaledMax.x) ? scaledMax.x
                                                   : newCenter.x;
    closestPoint.y = (newCenter.y < scaledMin.y)   ? scaledMin.y
                     : (newCenter.y > scaledMax.y) ? scaledMax.y
                                                   : newCenter.y;
    closestPoint.z = (newCenter.z < scaledMin.z)   ? scaledMin.z
                     : (newCenter.z > scaledMax.z) ? scaledMax.z
                                                   : newCenter.z;

    hitInfo.normal = glm::vec3(noScale * (newCenter - closestPoint));
    hitInfo.depth = other.getRadius() - glm::length(hitInfo.normal);
    hitInfo.normal = (hitInfo.depth <= glm::epsilon<float>()) ? hitInfo.normal : glm::normalize(hitInfo.normal);

    return hitInfo.depth > 0;
}

glm::vec4 CmxCuboid::getMin(const glm::mat4 &mat4) const
{
    return mat4 * glm::vec4{-1.f, -1.f, -1.f, 1.f};
}

glm::vec4 CmxCuboid::getMax(const glm::mat4 &mat4) const
{
    return mat4 * glm::vec4{1.f, 1.f, 1.f, 1.f};
}

} // namespace cmx
