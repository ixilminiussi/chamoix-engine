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

    assetsManager->getModel(PRIMITIVE_SPHERE)->bind(frameInfo.commandBuffer);
    assetsManager->getModel(PRIMITIVE_SPHERE)->draw(frameInfo.commandBuffer);
}

bool CmxSphere::overlapsWith(const CmxShape &other) const
{
    return other.overlapsWith(*this);
}

bool CmxSphere::overlapsWith(const CmxSphere &other) const
{
    return (glm::length(getCenter() - other.getCenter()) <= (getRadius() + other.getRadius()));
}

bool CmxSphere::overlapsWith(const CmxCuboid &other) const
{
    return other.overlapsWith(*this);
}

bool CmxSphere::overlapsWith(const CmxContainer &other) const
{
    return other.overlapsWith(*this);
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

glm::vec3 CmxPolygon::getNormal()
{
    return glm::normalize(glm::cross(*b - *a, *c - *a));
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

    assetsManager->getModel(PRIMITIVE_CUBE)->bind(frameInfo.commandBuffer);
    assetsManager->getModel(PRIMITIVE_CUBE)->draw(frameInfo.commandBuffer);
}

bool CmxCuboid::overlapsWith(const CmxShape &other) const
{
    return other.overlapsWith(*this);
}

bool CmxCuboid::overlapsWith(const CmxCuboid &other) const
{
    Transform transform = getAbsoluteTransform();

    const glm::mat4 mat4 = transform.mat4();
    const glm::mat4 inverseMat4 = glm::inverse(mat4);

    glm::vec4 p = other.getA(inverseMat4);
    float minX = p.x, minY = p.y, minZ = p.z, maxX = p.x, maxY = p.y, maxZ = p.z;

    auto update = [&minX, &minY, &minZ, &maxX, &maxY, &maxZ](glm::vec4 p) {
        minX = std::min(minX, p.x);
        minY = std::min(minY, p.y);
        minZ = std::min(minZ, p.z);
        maxX = std::max(maxX, p.x);
        maxY = std::max(maxY, p.y);
        maxZ = std::max(maxZ, p.z);
    };

    update(other.getB(inverseMat4));
    update(other.getC(inverseMat4));
    update(other.getD(inverseMat4));
    update(other.getE(inverseMat4));
    update(other.getF(inverseMat4));
    update(other.getG(inverseMat4));
    update(other.getH(inverseMat4));

    if (maxX < std::min(getA(mat4).x, getG(mat4).x))
        return false;
    if (maxY < std::min(getA(mat4).y, getG(mat4).y))
        return false;
    if (maxZ < std::min(getA(mat4).z, getG(mat4).z))
        return false;
    if (minX > std::max(getA(mat4).x, getG(mat4).x))
        return false;
    if (minY > std::max(getA(mat4).y, getG(mat4).y))
        return false;
    if (minZ > std::max(getA(mat4).z, getG(mat4).z))
        return false;

    return true;
}

bool CmxCuboid::overlapsWith(const CmxSphere &other) const
{
    Transform transform = getAbsoluteTransform();

    glm::mat4 scaler = glm::scale(glm::mat4(1.0f), transform.scale);

    glm::vec4 scaledA = getA(scaler);
    glm::vec4 scaledG = getG(scaler);

    glm::vec4 newP = glm::inverse(transform.mat4_noScale()) * glm::vec4{other.getCenter(), 1.0f};

    float radius = other.getRadius();

    if (newP.x + radius < std::min(scaledA.x, scaledG.x))
        return false;
    if (newP.x - radius > std::max(scaledA.x, scaledG.x))
        return false;

    if (newP.y + radius < std::min(scaledA.y, scaledG.y))
        return false;
    if (newP.y - radius > std::max(scaledA.y, scaledG.y))
        return false;

    if (newP.z + radius < std::min(scaledA.z, scaledG.z))
        return false;
    if (newP.z - radius > std::max(scaledA.z, scaledG.z))
        return false;

    return true;
}

bool CmxCuboid::overlapsWith(const CmxContainer &other) const
{
    return other.overlapsWith(*this);
}

glm::vec4 CmxCuboid::getA(const glm::mat4 &mat4) const
{
    return mat4 * glm::vec4{-1.f, -1.f, -1.f, 1.f};
}
glm::vec4 CmxCuboid::getB(const glm::mat4 &mat4) const
{
    return mat4 * glm::vec4{1.f, -1.f, -1.f, 1.f};
}
glm::vec4 CmxCuboid::getC(const glm::mat4 &mat4) const
{
    return mat4 * glm::vec4{1.f, -1.f, 1.f, 1.f};
}
glm::vec4 CmxCuboid::getD(const glm::mat4 &mat4) const
{
    return mat4 * glm::vec4{-1.f, -1.f, 1.f, 1.f};
}
glm::vec4 CmxCuboid::getE(const glm::mat4 &mat4) const
{
    return mat4 * glm::vec4{-1.f, 1.f, -1.f, 1.f};
}
glm::vec4 CmxCuboid::getF(const glm::mat4 &mat4) const
{
    return mat4 * glm::vec4{1.f, 1.f, -1.f, 1.f};
}
glm::vec4 CmxCuboid::getG(const glm::mat4 &mat4) const
{
    return mat4 * glm::vec4{1.f, 1.f, 1.f, 1.f};
}
glm::vec4 CmxCuboid::getH(const glm::mat4 &mat4) const
{
    return mat4 * glm::vec4{-1.f, 1.f, 1.f, 1.f};
}

CmxContainer::CmxContainer(Transformable *parent) : CmxCuboid{parent}
{
}

std::string CmxContainer::getName() const
{
    return PRIMITIVE_CONTAINER;
}

void CmxContainer::render(const class FrameInfo &frameInfo, VkPipelineLayout pipelineLayout,
                          class AssetsManager *assetsManager)
{
    EdgePushConstantData push{};

    Transform transform = getAbsoluteTransform();

    push.modelMatrix = transform.mat4();
    push.color = isOverlapping() ? glm::vec3{1.f, 0.f, 0.f} : glm::vec3{0.f, 1.f, 1.f};

    vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(EdgePushConstantData),
                       &push);

    assetsManager->getModel(PRIMITIVE_CONTAINER)->bind(frameInfo.commandBuffer);
    assetsManager->getModel(PRIMITIVE_CONTAINER)->draw(frameInfo.commandBuffer);
}

bool CmxContainer::overlapsWith(const CmxShape &other) const
{
    return other.overlapsWith(*this);
}

bool CmxContainer::overlapsWith(const CmxCuboid &other) const
{
    Transform transform = getAbsoluteTransform();

    const glm::mat4 mat4 = transform.mat4();
    const glm::mat4 inverseMat4 = glm::inverse(mat4);

    glm::vec4 p = other.getA(inverseMat4);
    float minX = p.x, minY = p.y, minZ = p.z, maxX = p.x, maxY = p.y, maxZ = p.z;

    auto update = [&minX, &minY, &minZ, &maxX, &maxY, &maxZ](glm::vec4 p) {
        minX = std::min(minX, p.x);
        minY = std::min(minY, p.y);
        minZ = std::min(minZ, p.z);
        maxX = std::max(maxX, p.x);
        maxY = std::max(maxY, p.y);
        maxZ = std::max(maxZ, p.z);
    };

    update(other.getB(inverseMat4));
    update(other.getC(inverseMat4));
    update(other.getD(inverseMat4));
    update(other.getE(inverseMat4));
    update(other.getF(inverseMat4));
    update(other.getG(inverseMat4));
    update(other.getH(inverseMat4));

    if (minX < std::min(getA(mat4).x, getG(mat4).x))
        return true;
    if (minY < std::min(getA(mat4).y, getG(mat4).y))
        return true;
    if (minZ < std::min(getA(mat4).z, getG(mat4).z))
        return true;
    if (maxX > std::max(getA(mat4).x, getG(mat4).x))
        return true;
    if (maxY > std::max(getA(mat4).y, getG(mat4).y))
        return true;
    if (maxZ > std::max(getA(mat4).z, getG(mat4).z))
        return true;

    return false;
}

bool CmxContainer::overlapsWith(const CmxSphere &other) const
{
    Transform transform = getAbsoluteTransform();

    glm::mat4 scaler = glm::scale(glm::mat4(1.0f), transform.scale);

    glm::vec4 scaledA = getA(scaler);
    glm::vec4 scaledG = getG(scaler);

    glm::vec4 newP = glm::inverse(transform.mat4_noScale()) * glm::vec4{other.getCenter(), 1.0f};

    float radius = other.getRadius();

    if (newP.x + radius > std::max(scaledA.x, scaledG.x))
        return true;
    if (newP.x - radius < std::min(scaledA.x, scaledG.x))
        return true;

    if (newP.y + radius > std::max(scaledA.y, scaledG.y))
        return true;
    if (newP.y - radius < std::min(scaledA.y, scaledG.y))
        return true;

    if (newP.z + radius > std::max(scaledA.z, scaledG.z))
        return true;
    if (newP.z - radius < std::min(scaledA.z, scaledG.z))
        return true;

    return false;
}

bool CmxContainer::overlapsWith(const CmxContainer &other) const
{
    return true;
}

} // namespace cmx
