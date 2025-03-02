#include "cmx_shapes.h"

// cmx
#include "cmx_assets_manager.h"
#include "cmx_edge_render_system.h"
#include "cmx_frame_info.h"
#include "cmx_math.h"
#include "cmx_model.h"
#include "cmx_physics_actor.h"
#include "cmx_physics_component.h"
#include "cmx_primitives.h"

// lib
#include "imgui.h"
#include <glm/ext/matrix_projection.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/geometric.hpp>
#include <glm/matrix.hpp>
#include <limits>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>

// std
#include <limits.h>

namespace cmx
{

Shape::Shape(cmx::Transformable *parent) : _parent{parent}
{
    _overlappingComponents[0] = {};
    _overlappingComponents[1] = {};
}

Transform Shape::getWorldSpaceTransform() const
{
    if (_parent)
    {
        return _parent->getWorldSpaceTransform();
    }
    return Transform::ONE;
}

const Transform &Shape::getLocalSpaceTransform() const
{
    return Transform::ONE;
}

bool Shape::wasOverlapping(PhysicsBody *component) const
{
    auto search = _overlappingComponents[_alternativeBuffer].find(component);

    return (search != _overlappingComponents[_alternativeBuffer].end());
}

bool Shape::isOverlapping(PhysicsBody *component) const
{
    auto search = _overlappingComponents[_buffer].find(component);

    return (search != _overlappingComponents[_buffer].end());
}

bool Shape::isOverlapping() const
{
    return !_overlappingComponents[_buffer].empty();
}

void Shape::addOverlappingComponent(PhysicsBody *component)
{
    _overlappingComponents[_buffer].emplace(component);
}

void Shape::reassess()
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
            if (auto ourComponent = dynamic_cast<PhysicsBody *>(_parent))
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

void Shape::swapBuffer()
{
    _alternativeBuffer = _buffer;
    _buffer = (_buffer + 1) % 2;
    _overlappingComponents[_buffer].clear();
}

Sphere::Sphere(cmx::Transformable *parent) : Shape{parent}
{
}

std::string Sphere::getName() const
{
    return PRIMITIVE_SPHERE;
}

void Sphere::render(const FrameInfo &frameInfo, vk::PipelineLayout pipelineLayout, AssetsManager *assetsManager)
{
    EdgePushConstantData push{};

    Transform transform = getWorldSpaceTransform();
    transform.scale.x = getRadius();
    transform.scale.y = transform.scale.x;
    transform.scale.z = transform.scale.y;

    push.modelMatrix = transform.mat4();
    push.color = isOverlapping() ? glm::vec3{1.f, 0.f, 0.f} : glm::vec3{0.f, 1.f, 1.f};

    vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(EdgePushConstantData),
                       &push);

    if (Model *model = assetsManager->getModel(PRIMITIVE_SPHERE))
    {
        model->bind(frameInfo.commandBuffer);
        model->draw(frameInfo.commandBuffer);
    }
}

bool Sphere::overlapsWith(const Shape &other, HitInfo &hitInfo) const
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

bool Sphere::overlapsWith(const Sphere &other, HitInfo &hitInfo) const
{
    if (!(mask & other.mask))
        return false;

    glm::vec3 center = getCenter();
    float radius = getRadius();

    hitInfo.normal = (other.getCenter() - center);
    float minDist = radius + other.getRadius();
    hitInfo.depth = minDist - glm::length(hitInfo.normal);
    hitInfo.normal = (hitInfo.depth <= glm::epsilon<float>()) ? hitInfo.normal : glm::normalize(hitInfo.normal);
    hitInfo.point = center + (radius * -hitInfo.normal);

    return hitInfo.depth > 0.f;
}

bool Sphere::overlapsWith(const Plane &other, HitInfo &hitInfo) const
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

bool Sphere::overlapsWith(const Cuboid &other, HitInfo &hitInfo) const
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

glm::mat3 Sphere::getInertiaTensor() const
{
    glm::mat3 tensor{0.f};

    const float radius = getRadius();
    tensor[0][0] = 2.0f * radius * radius / 5.0f;
    tensor[1][1] = 2.0f * radius * radius / 5.0f;
    tensor[2][2] = 2.0f * radius * radius / 5.0f;

    return tensor;
}

glm::vec3 Sphere::getCenter() const
{
    return getWorldSpaceTransform().position;
}

float Sphere::getRadius() const
{
    Transform transform = getWorldSpaceTransform();

    return std::max(std::max(transform.scale.x, transform.scale.y), transform.scale.z);
}

Cuboid::Cuboid(cmx::Transformable *parent) : Shape{parent}
{
}

std::string Cuboid::getName() const
{
    return PRIMITIVE_CUBE;
}

void Cuboid::render(const FrameInfo &frameInfo, vk::PipelineLayout pipelineLayout, AssetsManager *assetsManager)
{
    EdgePushConstantData push{};

    Transform transform = getWorldSpaceTransform();

    push.modelMatrix = transform.mat4();
    push.color = isOverlapping() ? glm::vec3{1.f, 0.f, 0.f} : glm::vec3{0.f, 1.f, 1.f};

    frameInfo.commandBuffer.pushConstants(pipelineLayout,
                                          vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0,
                                          sizeof(EdgePushConstantData), &push);

    if (Model *model = assetsManager->getModel(PRIMITIVE_CUBE))
    {
        model->bind(frameInfo.commandBuffer);
        model->draw(frameInfo.commandBuffer);
    }
}

bool Cuboid::overlapsWith(const Shape &other, HitInfo &hitInfo) const
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

std::vector<glm::vec3> Cuboid::getVerticesWorldSpace() const
{
    std::vector<glm::vec3> vertices;
    vertices.reserve(8);

    vertices.push_back(getMinWorldSpace());
    vertices.push_back(getMaxWorldSpace());
    vertices.emplace_back(vertices[0].x, vertices[0].y, vertices[1].z);
    vertices.emplace_back(vertices[0].x, vertices[1].y, vertices[0].z);
    vertices.emplace_back(vertices[0].x, vertices[1].y, vertices[1].z);
    vertices.emplace_back(vertices[1].x, vertices[0].y, vertices[0].z);
    vertices.emplace_back(vertices[1].x, vertices[0].y, vertices[1].z);
    vertices.emplace_back(vertices[1].x, vertices[1].y, vertices[0].z);

    return vertices;
}

std::pair<float, float> Cuboid::projectOnto(const glm::vec3 &vec) const
{
    const std::vector<glm::vec3> points = getVerticesWorldSpace();

    float min = std::numeric_limits<float>::infinity();
    float max = -std::numeric_limits<float>::infinity();

    for (const glm::vec3 &point : points)
    {
        const float distAlongRay = glm::dot(vec, project(point, vec));
        min = std::min(min, distAlongRay);
        max = std::max(max, distAlongRay);
    }

    return {min, max};
}

glm::vec3 Cuboid::getSupportPoint(const glm::vec3 &direction) const
{
    const std::vector<glm::vec3> vertices = getVerticesWorldSpace();
    glm::vec3 bestPoint = vertices[0];
    float maxDot = glm::dot(vertices[0], direction);

    for (const auto &vertex : vertices)
    {
        float dotProduct = glm::dot(vertex, direction);
        if (dotProduct > maxDot)
        {
            maxDot = dotProduct;
            bestPoint = vertex;
        }
    }

    return bestPoint;
}

bool Cuboid::overlapsWith(const Cuboid &other, HitInfo &hitInfo) const
{
    if (!(mask & other.mask))
        return false;

    glm::vec3 toBeTested[15] = {};

    // first we get all the possible axes needed
    // our 3
    const glm::mat4 us_mat4 = getWorldSpaceTransform().mat4();
    toBeTested[0] = us_mat4 * glm::vec4(getFaceNormal({{-1, -1, -1}, {1, -1, -1}, {1, -1, 1}}), 1.0f);
    toBeTested[1] = us_mat4 * glm::vec4(getFaceNormal({{-1, -1, -1}, {-1, 1, -1}, {1, 1, -1}}), 1.0f);
    toBeTested[2] = us_mat4 * glm::vec4(getFaceNormal({{-1, -1, -1}, {-1, -1, 1}, {-1, 1, 1}}), 1.0f);

    // their 3
    const glm::mat4 them_mat4 = other.getWorldSpaceTransform().mat4();
    toBeTested[3] = them_mat4 * glm::vec4(getFaceNormal({{-1, -1, -1}, {1, -1, -1}, {1, -1, 1}}), 1.0f);
    toBeTested[4] = them_mat4 * glm::vec4(getFaceNormal({{-1, -1, -1}, {-1, 1, -1}, {1, 1, -1}}), 1.0f);
    toBeTested[5] = them_mat4 * glm::vec4(getFaceNormal({{-1, -1, -1}, {-1, -1, 1}, {-1, 1, 1}}), 1.0f);

    // cross
    toBeTested[6] = glm::cross(toBeTested[0], toBeTested[3]);
    toBeTested[7] = glm::cross(toBeTested[0], toBeTested[4]);
    toBeTested[8] = glm::cross(toBeTested[0], toBeTested[6]);
    toBeTested[9] = glm::cross(toBeTested[1], toBeTested[3]);
    toBeTested[10] = glm::cross(toBeTested[1], toBeTested[4]);
    toBeTested[11] = glm::cross(toBeTested[1], toBeTested[6]);
    toBeTested[12] = glm::cross(toBeTested[2], toBeTested[3]);
    toBeTested[13] = glm::cross(toBeTested[2], toBeTested[4]);
    toBeTested[14] = glm::cross(toBeTested[2], toBeTested[6]);

    // then we project the points onto it
    hitInfo.depth = std::numeric_limits<float>::infinity();

    for (glm::vec3 &vec : toBeTested)
    {
        if (vec.length() < glm::epsilon<float>())
            continue;

        vec = glm::normalize(vec);

        auto [us_min, us_max] = projectOnto(vec);
        auto [them_min, them_max] = other.projectOnto(vec);

        const float overlapStart = std::max(us_min, them_min);
        const float overlapEnd = std::min(us_max, them_max);
        const float overlap = overlapEnd - overlapStart;

        if (overlap <= 0)
        {
            return false;
        }
        if (overlap < hitInfo.depth)
        {
            hitInfo.depth = overlap;
            hitInfo.normal = vec;
        }
    }

    return true;
}

bool Cuboid::overlapsWith(const Plane &other, HitInfo &hitInfo) const
{
    if (!(mask & other.mask))
        return false;

    glm::vec3 toBeTested[7] = {};

    // first we get all the possible axes needed
    // our 3
    const glm::mat4 us_mat4 = getWorldSpaceTransform().mat4();
    toBeTested[0] = us_mat4 * glm::vec4(getFaceNormal({{-1, -1, -1}, {1, -1, -1}, {1, -1, 1}}), 1.0f);
    toBeTested[1] = us_mat4 * glm::vec4(getFaceNormal({{-1, -1, -1}, {-1, 1, -1}, {1, 1, -1}}), 1.0f);
    toBeTested[2] = us_mat4 * glm::vec4(getFaceNormal({{-1, -1, -1}, {-1, -1, 1}, {-1, 1, 1}}), 1.0f);

    // their 3
    const glm::mat4 them_mat4 = other.getWorldSpaceTransform().mat4();
    toBeTested[3] = them_mat4 * glm::vec4(getFaceNormal({{-1, 0, -1}, {1, 0, -1}, {1, 0, 1}}), 1.0f);

    // cross
    toBeTested[4] = glm::cross(toBeTested[0], toBeTested[3]);
    toBeTested[5] = glm::cross(toBeTested[1], toBeTested[3]);
    toBeTested[6] = glm::cross(toBeTested[2], toBeTested[3]);

    // then we project the points onto it
    hitInfo.depth = std::numeric_limits<float>::infinity();

    for (glm::vec3 &vec : toBeTested)
    {
        if (vec.length() < glm::epsilon<float>())
            continue;

        vec = glm::normalize(vec);

        auto [us_min, us_max] = projectOnto(vec);
        auto [them_min, them_max] = other.projectOnto(vec);

        const float overlapStart = std::max(us_min, them_min);
        const float overlapEnd = std::min(us_max, them_max);
        const float overlap = overlapEnd - overlapStart;

        if (overlap <= 0)
        {
            return false;
        }
        if (overlap < hitInfo.depth)
        {
            hitInfo.depth = overlap;
            hitInfo.normal = vec;
        }
    }

    return true;
}

std::vector<glm::vec3> Plane::getVerticesWorldSpace() const
{
    std::vector<glm::vec3> vertices;
    vertices.reserve(4);

    vertices.push_back(getMinWorldSpace());
    vertices.push_back(getMaxWorldSpace());
    vertices.emplace_back(vertices[0].x, vertices[0].y, vertices[1].z);
    vertices.emplace_back(vertices[1].x, vertices[0].y, vertices[0].z);

    return vertices;
}

std::pair<float, float> Plane::projectOnto(const glm::vec3 &vec) const
{
    std::vector<glm::vec3> points = getVerticesWorldSpace();

    float min = std::numeric_limits<float>::infinity();
    float max = -std::numeric_limits<float>::infinity();

    for (const glm::vec3 &point : points)
    {
        const float distAlongRay = glm::dot(vec, project(point, vec));
        min = std::min(min, distAlongRay);
        max = std::max(max, distAlongRay);
    }

    return {min, max};
}

bool Cuboid::overlapsWith(const Sphere &other, HitInfo &hitInfo) const
{
    const Transform transform = getWorldSpaceTransform();

    const glm::vec4 scaledMin = getMinLocalSpace();
    const glm::vec4 scaledMax = getMaxLocalSpace();

    const glm::mat4 noScale = transform.mat4_noScale();
    const glm::vec4 newCenter = glm::inverse(noScale) * glm::vec4{other.getCenter(), 1.0f};

    // OBB
    // get closest point
    glm::vec3 closestPoint{1.f};
    closestPoint.x = (newCenter.x < scaledMin.x)   ? scaledMin.x
                     : (newCenter.x > scaledMax.x) ? scaledMax.x
                                                   : newCenter.x;
    closestPoint.y = (newCenter.y < scaledMin.y)   ? scaledMin.y
                     : (newCenter.y > scaledMax.y) ? scaledMax.y
                                                   : newCenter.y;
    closestPoint.z = (newCenter.z < scaledMin.z)   ? scaledMin.z
                     : (newCenter.z > scaledMax.z) ? scaledMax.z
                                                   : newCenter.z;

    hitInfo.normal = glm::vec3(noScale * (newCenter - glm::vec4(closestPoint, 1.0f)));
    hitInfo.depth = other.getRadius() - glm::length(hitInfo.normal);
    hitInfo.normal = (hitInfo.depth <= glm::epsilon<float>()) ? hitInfo.normal : glm::normalize(hitInfo.normal);
    hitInfo.point = glm::vec3(noScale * glm::vec4(closestPoint, 1.0f));

    return hitInfo.depth > 0;
}

glm::mat3 Cuboid::getInertiaTensor() const
{
    const glm::vec3 dimensions = getMaxLocalSpace() - getMinLocalSpace();

    glm::mat3 mat3{0.f};
    mat3[0][0] = (1.0f / 12.0f) * (dimensions.y * dimensions.y + dimensions.z * dimensions.z);
    mat3[1][1] = (1.0f / 12.0f) * (dimensions.x * dimensions.x + dimensions.z * dimensions.z);
    mat3[2][2] = (1.0f / 12.0f) * (dimensions.x * dimensions.x + dimensions.y * dimensions.y);

    return mat3;
}

glm::vec4 Cuboid::getMinLocalSpace() const
{
    const glm::mat4 scaler = glm::scale(glm::mat4(1.0f), getWorldSpaceTransform().scale);
    return scaler * glm::vec4{-1.f, -1.f, -1.f, 1.f};
}

glm::vec4 Cuboid::getMaxLocalSpace() const
{
    const glm::mat4 scaler = glm::scale(glm::mat4(1.0f), getWorldSpaceTransform().scale);
    return scaler * glm::vec4{1.f, 1.f, 1.f, 1.f};
}

glm::vec4 Cuboid::getMinWorldSpace() const
{
    const Transform transform = getWorldSpaceTransform();
    const glm::mat4 noScale = transform.mat4_noScale();

    return noScale * getMinLocalSpace();
}

glm::vec4 Cuboid::getMaxWorldSpace() const
{
    const Transform transform = getWorldSpaceTransform();
    const glm::mat4 noScale = transform.mat4_noScale();

    return noScale * getMaxLocalSpace();
}

Plane::Plane(cmx::Transformable *parent) : Cuboid{parent}
{
}

std::string Plane::getName() const
{
    return PRIMITIVE_PLANE;
}

void Plane::render(const FrameInfo &frameInfo, vk::PipelineLayout pipelineLayout, AssetsManager *assetsManager)
{
    EdgePushConstantData push{};

    Transform transform = getWorldSpaceTransform();

    push.modelMatrix = transform.mat4();
    push.color = isOverlapping() ? glm::vec3{1.f, 0.f, 0.f} : glm::vec3{0.f, 1.f, 1.f};

    vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(EdgePushConstantData),
                       &push);

    if (Model *model = assetsManager->getModel(PRIMITIVE_PLANE))
    {
        model->bind(frameInfo.commandBuffer);
        model->draw(frameInfo.commandBuffer);
    }
}

bool Plane::overlapsWith(const Plane &other, HitInfo &hitInfo) const
{
    return false;
}

bool Plane::overlapsWith(const Cuboid &other, HitInfo &hitInfo) const
{
    bool b = other.overlapsWith(*this, hitInfo);

    if (b)
    {
        hitInfo.flip();
        return true;
    }

    return false;
}

glm::vec4 Plane::getMinLocalSpace() const
{
    const glm::mat4 scaler = glm::scale(glm::mat4(1.0f), getWorldSpaceTransform().scale);
    return scaler * glm::vec4{-1.f, 0.f, -1.f, 1.f};
}

glm::vec4 Plane::getMaxLocalSpace() const
{
    const glm::mat4 scaler = glm::scale(glm::mat4(1.0f), getWorldSpaceTransform().scale);
    return scaler * glm::vec4{1.f, 0.f, 1.f, 1.f};
}

} // namespace cmx
