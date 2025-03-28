#include "cmx_shapes.h"

// cmx
#include "cmx_assets_manager.h"
#include "cmx_bounds.h"
#include "cmx_math.h"
#include "cmx_model.h"
#include "cmx_physics_actor.h"
#include "cmx_physics_util.h"
#include "cmx_primitives.h"
#include "cmx_render_system.h"

// lib
#include <glm/ext/matrix_projection.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/geometric.hpp>
#include <glm/matrix.hpp>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_structs.hpp>

// std
#include <algorithm>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace cmx
{

Shape::Shape(cmx::Transformable *parent) : _parent{parent}
{
    _overlappingComponents[0] = {};
    _overlappingComponents[1] = {};
}

const Transform &Shape::getLocalSpaceTransform() const
{
    return Transform::ONE;
}

Transform Shape::getWorldSpaceTransform(int depth) const
{
    if (_parent == nullptr || depth == 0)
    {
        return getLocalSpaceTransform();
    }

    if (depth == -1)
    {
        return getLocalSpaceTransform() + _parent->getWorldSpaceTransform();
    }

    return getLocalSpaceTransform() + _parent->getWorldSpaceTransform(depth - 1);
}

void Shape::setMask(uint8_t mask)
{
    this->mask = mask;
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
                if (auto ourActor = dynamic_cast<PhysicsActor *>(ourComponent->getParentActor()))
                {
                    if (*previousIt != nullptr)
                    {
                        ourActor->onEndOverlap(ourComponent, *previousIt, (*previousIt)->getParentActor());
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

Model *Sphere::getModel(AssetsManager *assetsManager) const
{
    if (assetsManager == nullptr)
    {
        return nullptr;
    }

    return assetsManager->getModel(PRIMITIVE_SPHERE);
}

std::string Sphere::getName() const
{
    return PRIMITIVE_SPHERE;
}

glm::vec3 Sphere::getCenterOfMass() const
{
    return glm::vec3{0.f};
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

bool Sphere::overlapsWith(const Convex &other, HitInfo &hitInfo) const
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

    return std::max({transform.scale.x, transform.scale.y, transform.scale.z});
}

glm::vec3 Sphere::support(const glm::vec3 &dir, const float bias) const
{
    return getCenter() + dir * (getRadius() + bias);
}

bool Convex::overlapsWith(const Sphere &other, HitInfo &hitInfo) const
{
    return overlapsWith(static_cast<const Shape &>(other), hitInfo);
}

bool Convex::overlapsWith(const Convex &other, HitInfo &hitInfo) const
{
    return overlapsWith(static_cast<const Shape &>(other), hitInfo);
}

bool Convex::overlapsWith(const Shape &other, HitInfo &hitInfo) const
{
    if (!(mask & other.mask))
        return false;

    const Transform otherTransform = other.getWorldSpaceTransform();
    const Transform selfTransform = getWorldSpaceTransform();
    static auto supportGJK = [](const Shape &a, const Shape &b, const glm::vec3 &d) {
        SupportPoint point;
        point.a = a.support(d, 0.f);
        point.b = b.support(-d, 0.f);
        point.xyz = point.a - point.b;

        return point;
    };

    glm::vec3 d = otherTransform.position - selfTransform.position;

    std::array<SupportPoint, 4> simplex;
    simplex[0] = supportGJK(*this, other, d);
    int numPoints = 1;

    d = -simplex[0].xyz;

    bool isColliding = false;
    float closestDist = std::numeric_limits<float>::infinity();
    while (!isColliding)
    {
        SupportPoint A = supportGJK(*this, other, d);

        if (simplexHasPoint(simplex, numPoints, A))
        {
            break;
        }

        simplex[numPoints] = A;
        numPoints++;

        float dotdot = glm::dot(d, A.xyz);
        if (dotdot < 0.0f)
        {
            break;
        }

        glm::vec4 lambdas;
        if (simplexSignedVolumes(simplex, numPoints, d, lambdas))
        {
            isColliding = true;
            break;
        }

        float dist = glm::length2(d);
        if (dist >= closestDist)
        {
            break;
        }
        closestDist = dist;

        sortValids(simplex, lambdas);

        int numPoints = 0;
        for (int i = 0; i < 4; i++)
        {
            if (0.0f != lambdas[i])
            {
                numPoints++;
            }
        }

        isColliding = (numPoints == 4);
    }

    if (!isColliding)
        return false;

    spdlog::critical("hihihi");

    if (numPoints == 1)
    {
        glm::vec3 searchDir = simplex[0].xyz * -1.0f;
        SupportPoint newPoint = supportGJK(*this, other, searchDir);
        simplex[numPoints] = newPoint;
        numPoints++;
    }
    if (numPoints == 2)
    {
        glm::vec3 ab = simplex[1].xyz - simplex[0].xyz;
        glm::vec3 u, v;
        getOrtho(ab, u, v);

        glm::vec3 newDir = u;
        SupportPoint newPoint = supportGJK(*this, other, newDir);
        simplex[numPoints] = newPoint;
        numPoints++;
    }
    if (numPoints == 3)
    {
        glm::vec3 ab = simplex[1].xyz - simplex[0].xyz;
        glm::vec3 ac = simplex[2].xyz - simplex[0].xyz;
        glm::vec3 norm = glm::cross(ab, ac);

        glm::vec3 newDir = norm;
        SupportPoint newPoint = supportGJK(*this, other, newDir);
        simplex[numPoints] = newPoint;
        numPoints++;
    }

    //
    // Expand the simplex by the bias amount
    //

    // Get the center point of the simplex
    glm::vec3 avg = glm::vec3(0, 0, 0);
    for (int i = 0; i < 4; i++)
    {
        avg += simplex[i].xyz;
    }
    avg *= 0.25f;

    // Now expand the simplex by the bias amount
    for (int i = 0; i < numPoints; i++)
    {
        SupportPoint &pt = simplex[i];

        glm::vec3 dir = pt.xyz - avg; // ray from "center" to witness point
        dir = glm::normalize(dir);
        pt.a += dir * 0.1f;
        pt.b -= dir * 0.1f;
        pt.xyz = pt.a - pt.b;
    }

    glm::vec3 pointOnA;
    glm::vec3 pointOnB;
    expandEPA(other, 0.001f, simplex, pointOnA, pointOnB);

    hitInfo.point = pointOnA;
    hitInfo.depth = glm::length(pointOnB - pointOnA);
    hitInfo.normal = glm::normalize(pointOnB - pointOnA);

    return true;
}

bool Convex::simplexSignedVolumes(const std::array<SupportPoint, 4> &simplex, const int numPoints, glm::vec3 &d,
                                  glm::vec4 &lambdasOut) const
{
    lambdasOut = glm::vec4{0.f};

    bool doesIntersect = false;

    switch (numPoints)
    {
    default:
    case 2: {
        glm::vec2 lambdas = signedVolume1D(simplex[0].xyz, simplex[1].xyz);
        glm::vec3 v(0.0f);
        for (int i = 0; i < 2; i++)
        {
            v += simplex[i].xyz * lambdas[i];
        }
        d = v * -1.0f;
        doesIntersect = (glm::length2(v) < glm::epsilon<float>());
        lambdasOut[0] = lambdas[0];
        lambdasOut[1] = lambdas[1];
    }
    break;
    case 3: {
        glm::vec3 lambdas = signedVolume2D(simplex[0].xyz, simplex[1].xyz, simplex[2].xyz);
        glm::vec3 v(0.0f);
        for (int i = 0; i < 3; i++)
        {
            v += simplex[i].xyz * lambdas[i];
        }
        d = v * -1.0f;
        doesIntersect = (glm::length2(v) < glm::epsilon<float>());
        lambdasOut[0] = lambdas[0];
        lambdasOut[1] = lambdas[1];
        lambdasOut[2] = lambdas[2];
    }
    break;
    case 4: {
        glm::vec4 lambdas = signedVolume3D(simplex[0].xyz, simplex[1].xyz, simplex[2].xyz, simplex[3].xyz);
        glm::vec3 v(0.0f);
        for (int i = 0; i < 4; i++)
        {
            v += simplex[i].xyz * lambdas[i];
        }
        d = v * -1.0f;
        doesIntersect = (glm::length2(v) < glm::epsilon<float>());
        lambdasOut[0] = lambdas[0];
        lambdasOut[1] = lambdas[1];
        lambdasOut[2] = lambdas[2];
        lambdasOut[3] = lambdas[3];
    }
    break;
    };

    return doesIntersect;
}

void Convex::sortValids(std::array<SupportPoint, 4> &simplexPoints, glm::vec4 &lambdas) const
{
    std::array<bool, 4> valids;
    for (int i = 0; i < 4; i++)
    {
        valids[i] = lambdas[i] != 0.0f;
    }

    glm::vec4 validLambdas(0.0f);
    int validCount = 0;
    std::array<SupportPoint, 4> validPts;

    memset(&validPts, 0, sizeof(std::array<SupportPoint, 4>));

    for (int i = 0; i < 4; i++)
    {
        if (valids[i])
        {
            validPts[validCount] = simplexPoints[i];
            validLambdas[validCount] = lambdas[i];
            validCount++;
        }
    }

    for (int i = 0; i < 4; i++)
    {
        simplexPoints[i] = validPts[i];
        lambdas[i] = validLambdas[i];
    }
}

bool Convex::simplexHasPoint(const std::array<SupportPoint, 4> &simplex, int numPoints, const SupportPoint &point) const
{
    for (int i = 0; i < numPoints; i++)
    {
        if (point == simplex[i])
        {
            return true;
        }
    }
    return false;
};

Convex::Convex(cmx::Transformable *parent) : Shape{parent}
{
    static const float val1 = 2 / 4.f;
    static const float val2 = 2 / 3.5f;
    buildConvex({{0, val2, 0}, {0, -val2, val2}, {-val1, -val2, -val2}, {val1, -val2, -val2}});
}

Convex::~Convex()
{
    _model->free();
    delete _model;
    _model = nullptr;
}

void Convex::buildConvex(const std::vector<glm::vec3> &vertices)
{
    _vertices.clear();
    _faceIndices.clear();

    if (vertices.size() < 4)
    {
        throw std::runtime_error("Convex: attempt at building convex shape with less than 4 vertices");
    }

    std::array<glm::vec3, 4> simplex;

    int idx = findPointFurthestInDir(vertices, glm::vec3{1, 0, 0});
    simplex[0] = vertices[idx];
    idx = findPointFurthestInDir(vertices, -simplex[0]);
    simplex[1] = vertices[idx];
    idx = findPointFurthestFromLine(vertices, simplex[0], simplex[1]);
    simplex[2] = vertices[idx];
    idx = findPointFurthestFromFace(vertices, simplex[0], simplex[1], simplex[2]);
    simplex[3] = vertices[idx];

    float dist = getTripleProduct(simplex[0], simplex[1], simplex[2], simplex[3]);
    if (dist > 0.0f)
    {
        std::swap(simplex[0], simplex[1]);
    }

    _vertices.push_back(simplex[0]);
    _vertices.push_back(simplex[1]);
    _vertices.push_back(simplex[2]);
    _vertices.push_back(simplex[3]);

    _faceIndices.push_back({0, 1, 2});
    _faceIndices.push_back({0, 2, 3});
    _faceIndices.push_back({2, 1, 3});
    _faceIndices.push_back({1, 0, 3});

    for (const glm::vec3 &vertex : vertices)
    {
        expandConvex(vertex);
    }

    calculateCenterOfMass();
    calculateInertiaTensor();
    makeModel();
}

void Convex::makeModel()
{
    if (_model != nullptr)
    {
        _model->free();
        delete _model;
        _model = nullptr;
    }

    std::vector<int> indices;
    indices.reserve(_faceIndices.size() * 3);

    for (const Face &face : _faceIndices)
    {
        indices.emplace_back(face.a);
        indices.emplace_back(face.b);
        indices.emplace_back(face.c);
    }

    _model = Model::createModelFromVerticesAndIndices(RenderSystem::getInstance()->getDevice(), _vertices, indices);
}

glm::vec3 Convex::getCenterOfMass() const
{
    return _centerOfMass;
}

glm::mat3 Convex::getInertiaTensor() const
{
    return _tensor;
}

Model *Convex::getModel(AssetsManager *) const
{
    return _model;
}

std::string Convex::getName() const
{
    return "Convex";
}

std::vector<glm::vec3> Convex::getWorldSpaceVertices() const
{
    std::vector<glm::vec3> vertices;
    vertices.reserve(_vertices.size());

    const glm::mat4 mat4 = getWorldSpaceTransform().mat4();

    for (const glm::vec3 &vertex : vertices)
    {
        vertices.emplace_back(mat4 * glm::vec4(vertex, 1.f));
    }

    return vertices;
}

glm::vec3 Convex::support(const glm::vec3 &dir, const float bias) const
{
    std::vector<glm::vec3> worldVertices = getWorldSpaceVertices();
    glm::vec3 minPt;
    float minDst = std::numeric_limits<float>::infinity();

    for (const glm::vec3 &vertex : worldVertices)
    {
        const float dist = glm::dot(dir, vertex);

        if (dist < minDst)
        {
            minDst = dist;
            minPt = vertex;
        }
    }

    return minPt + (glm::normalize(dir) * bias);
}

std::array<Convex::Edge, 3> Convex::getEdges(const Face &face) const
{
    std::array<Edge, 3> edges;
    edges[0].a = face.a;
    edges[0].b = face.b;

    edges[1].a = face.b;
    edges[1].b = face.c;

    edges[2].a = face.c;
    edges[2].b = face.a;

    return edges;
}

bool Convex::isEdgeUnique(const std::vector<const Face *> &faces, const Edge &edge, int ignoreIdx) const
{
    for (int i = 0; i < faces.size(); i++)
    {
        if (ignoreIdx == i)
        {
            continue;
        }

        for (const Edge &other : getEdges(*faces[i]))
        {
            if (other.a == edge.a && other.b == edge.b)
            {
                return false;
            }
        }
    }

    return true;
}

void Convex::expandConvex(const glm::vec3 &vertex)
{
    if (isInternalVertex(vertex))
    {
        return;
    }

    for (const glm::vec3 &other : _vertices)
    {
        if (glm::length2(other - vertex) < 0.001f)
        {
            return;
        }
    }

    std::vector<const Face *> candidates;

    // collect all faces facing the vertex
    for (const Face &face : _faceIndices)
    {
        if (getTripleProduct(_vertices[face.a], _vertices[face.b], _vertices[face.c], vertex) > 0.f)
        {
            candidates.emplace_back(&face);
        }
    }

    // collect all edges unique amongst candidate faces (meaning they only appear once)
    // these will make the new faces
    std::vector<Edge> uniqueEdges;
    for (int i = 0; i < candidates.size(); i++)
    {
        const Face *face = candidates[i];
        std::array<Edge, 3> edges = getEdges(*face);

        for (int i = 0; i < 3; i++)
        {
            if (isEdgeUnique(candidates, edges[i], i))
            {
                uniqueEdges.emplace_back(edges[i]);
            }
        }
    }

    // Now remove the old facing tris
    for (const Face *pFace : candidates)
    {
        const auto it = std::find_if(_faceIndices.begin(), _faceIndices.end(),
                                     [pFace](const Face &face) { return &face == pFace; });

        _faceIndices.erase(it);
    }

    // Now add the new point
    _vertices.push_back(vertex);
    const int newIndex = (int)_vertices.size() - 1;

    for (auto edge : uniqueEdges)
    {
        Face face;
        face.a = edge.a;
        face.b = edge.b;
        face.c = newIndex;
        _faceIndices.push_back(face);
    }

    cleanInternalVertices();
}

int Convex::getClosestFace(const glm::vec3 &p, const std::vector<Face> &faceIndices,
                           const std::vector<SupportPoint> &points) const
{
    float minDistance = std::numeric_limits<float>::infinity();
    int idx = 0;

    int i = 0;
    for (const Face &face : faceIndices)
    {
        float signedDistance =
            glm::dot(points[face.a].xyz, getFaceNormal(points[face.a].xyz, points[face.b].xyz, points[face.c].xyz));

        if (signedDistance < minDistance)
        {
            idx = i;
            minDistance = signedDistance;
        }
        i++;
    }

    return idx;
}

int Convex::removeFacesFacingPoint(const glm::vec3 &p, std::vector<Face> &faces,
                                   std::vector<SupportPoint> &points) const
{
    int numRemoved = 0;
    for (int i = 0; i < faces.size(); i++)
    {
        const Face &face = faces[i];

        float dist = signedDistanceToFace(p, points[face.a].xyz, points[face.b].xyz, points[face.c].xyz);
        if (dist > 0.0f)
        {
            // This face faces the point.  Remove it.
            faces.erase(faces.begin() + i);
            i--;
            numRemoved++;
        }
    }
    return numRemoved;
}

void Convex::findDanglingEdges(std::vector<Edge> &danglingEdges, const std::vector<Face> &faces) const
{
    danglingEdges.clear();

    for (int i = 0; i < faces.size(); i++)
    {
        const Face &face = faces[i];

        std::array<Edge, 3> edges;
        edges[0].a = face.a;
        edges[0].b = face.b;

        edges[1].a = face.b;
        edges[1].b = face.c;

        edges[2].a = face.c;
        edges[2].b = face.a;

        std::array<int, 3> counts;
        counts[0] = 0;
        counts[1] = 0;
        counts[2] = 0;

        for (int j = 0; j < faces.size(); j++)
        {
            if (j == i)
            {
                continue;
            }

            const Face &face2 = faces[j];

            std::array<Edge, 3> edges2;
            edges2[0].a = face2.a;
            edges2[0].b = face2.b;

            edges2[1].a = face2.b;
            edges2[1].b = face2.c;

            edges2[2].a = face2.c;
            edges2[2].b = face2.a;

            for (int k = 0; k < 3; k++)
            {
                if (edges[k] == edges2[0])
                {
                    counts[k]++;
                }
                if (edges[k] == edges2[1])
                {
                    counts[k]++;
                }
                if (edges[k] == edges2[2])
                {
                    counts[k]++;
                }
            }
        }

        // An edge that isn't shared, is dangling
        for (int k = 0; k < 3; k++)
        {
            if (0 == counts[k])
            {
                danglingEdges.push_back(edges[k]);
            }
        }
    }
}

float Convex::expandEPA(const Shape &other, const float bias, const std::array<SupportPoint, 4> simplexPoints,
                        glm::vec3 &ptOnA, glm::vec3 &ptOnB) const
{
    std::vector<SupportPoint> points;
    std::vector<Face> faces;
    std::vector<Edge> danglingEdges;

    glm::vec3 center(0.0f);
    for (int i = 0; i < 4; i++)
    {
        points.push_back(simplexPoints[i]);
        center += simplexPoints[i].xyz;
    }
    center *= 0.25f;

    // Build the faces
    for (int i = 0; i < 4; i++)
    {
        int j = (i + 1) % 4;
        int k = (i + 2) % 4;
        Face face;
        face.a = i;
        face.b = j;
        face.c = k;

        int unusedPt = (i + 3) % 4;
        float dist =
            signedDistanceToFace(points[unusedPt].xyz, points[face.a].xyz, points[face.b].xyz, points[face.c].xyz);

        if (dist > 0.0f)
        {
            std::swap(face.a, face.b);
        }

        faces.push_back(face);
    }

    static auto supportGJK = [](const Shape &a, const Shape &b, const glm::vec3 &d, const float bias) {
        SupportPoint point;
        point.a = a.support(d, bias);
        point.b = b.support(-d, bias);
        point.xyz = point.a - point.b;

        return point;
    };

    while (true)
    {
        const int idx = getClosestFace(glm::vec3{0.f}, faces, points);
        glm::vec3 normal = getFaceNormal(points[faces[idx].a].xyz, points[faces[idx].b].xyz, points[faces[idx].c].xyz);

        const SupportPoint newPoint = supportGJK(*this, other, normal, bias);

        // if w already exists, then just stop
        // because it means we can't expand any further
        for (const Face &face : faces)
        {
            if (points[face.a] == newPoint || points[face.b] == newPoint || points[face.c] == newPoint)
            {
                goto skip; // break out of while loop
            }
        }

        float dist = signedDistanceToFace(newPoint.xyz, points[faces[idx].a].xyz, points[faces[idx].b].xyz,
                                          points[faces[idx].c].xyz);
        if (dist <= 0.0f)
        {
            break; // can't expand
        }

        const int newIdx = (int)points.size();
        points.push_back(newPoint);

        // Remove Faces that face this point
        int numRemoved = removeFacesFacingPoint(newPoint.xyz, faces, points);
        if (0 == numRemoved)
        {
            break;
        }

        // Find Dangling Edges
        danglingEdges.clear();
        findDanglingEdges(danglingEdges, faces);
        if (0 == danglingEdges.size())
        {
            break;
        }

        // In theory the edges should be a proper CCW order
        // So we only need to add the new point as 'a' in order
        // to create new faces that face away from origin
        for (auto edge : danglingEdges)
        {
            Face face;
            face.a = newIdx;
            face.b = edge.b;
            face.c = edge.a;

            // Make sure it's oriented properly
            float dist = signedDistanceToFace(center, points[face.a].xyz, points[face.b].xyz, points[face.c].xyz);
            if (dist > 0.0f)
            {
                std::swap(face.b, face.c);
            }

            faces.push_back(face);
        }
    }
skip:

    // Get the projection of the origin on the closest face
    const int idx = getClosestFace(glm::vec3{0.f}, faces, points);
    const Face &face = faces[idx];
    glm::vec3 ptA_w = points[face.a].xyz;
    glm::vec3 ptB_w = points[face.b].xyz;
    glm::vec3 ptC_w = points[face.c].xyz;
    glm::vec3 lambdas = BarycentricCoordinates(ptA_w, ptB_w, ptC_w, glm::vec3(0.0f));

    // Get the point on shape A
    glm::vec3 ptA_a = points[face.a].a;
    glm::vec3 ptB_a = points[face.b].a;
    glm::vec3 ptC_a = points[face.c].a;
    ptOnA = ptA_a * lambdas[0] + ptB_a * lambdas[1] + ptC_a * lambdas[2];

    // Get the point on shape B
    glm::vec3 ptA_b = points[face.a].b;
    glm::vec3 ptB_b = points[face.b].b;
    glm::vec3 ptC_b = points[face.c].b;
    ptOnB = ptA_b * lambdas[0] + ptB_b * lambdas[1] + ptC_b * lambdas[2];

    // Return the penetration distance
    glm::vec3 delta = ptOnB - ptOnA;
    return glm::length(delta);
}

bool Convex::isInternalVertex(const glm::vec3 &vertex) const
{
    for (auto face : _faceIndices)
    {
        const glm::vec3 &a = _vertices[face.a];
        const glm::vec3 &b = _vertices[face.b];
        const glm::vec3 &c = _vertices[face.c];

        float dist = getTripleProduct(a, b, c, vertex);
        if (dist > 0.0f)
        {
            return true;
        }
    }

    return false;
}

void Convex::cleanInternalVertices()
{
    for (int i = 0; i < _vertices.size(); i++)
    {
        const glm::vec3 &vertex = _vertices[i];

        if (isInternalVertex(vertex))
        {
            _vertices.erase(_vertices.begin() + i);
            i--;
        }
    }
}

void Convex::calculateCenterOfMass()
{
    float total = 0.f;
    glm::vec3 center = glm::vec3{0.f};

    for (const Face &face : _faceIndices)
    {
        const glm::vec3 &a = _vertices[face.a];
        const glm::vec3 &b = _vertices[face.b];
        const glm::vec3 &c = _vertices[face.c];

        float coefficient = getFaceVolume(a, b, c);
        center += getFaceCenter(a, b, c) * coefficient;
        total += coefficient;
    }

    _centerOfMass = center / total;
}

void Convex::calculateInertiaTensor()
{
    const int numSamples = 100;

    Bounds bounds;
    bounds.expand(_vertices);

    _tensor = glm::mat3{0.f};

    const float dx = bounds.getLengthX() / (float)numSamples;
    const float dy = bounds.getLengthY() / (float)numSamples;
    const float dz = bounds.getLengthZ() / (float)numSamples;

    int sampleCount = 0;
    for (float x = bounds.getMin().x; x < bounds.getMax().x; x += dx)
    {
        for (float y = bounds.getMin().y; y < bounds.getMax().y; y += dy)
        {
            for (float z = bounds.getMin().z; z < bounds.getMax().z; z += dz)
            {
                glm::vec3 point{x, y, z};

                if (!isInternalVertex(point))
                {
                    continue;
                }

                // Get the point relative to the center of mass
                point -= _centerOfMass;

                _tensor[0][0] += point.y * point.y + point.z * point.z;
                _tensor[1][1] += point.z * point.z + point.x * point.x;
                _tensor[2][2] += point.x * point.x + point.y * point.y;

                _tensor[1][0] += -1.0f * point.x * point.y;
                _tensor[2][0] += -1.0f * point.x * point.z;
                _tensor[2][1] += -1.0f * point.y * point.z;

                _tensor[0][1] += -1.0f * point.x * point.y;
                _tensor[0][2] += -1.0f * point.x * point.z;
                _tensor[1][2] += -1.0f * point.y * point.z;

                sampleCount++;
            }
        }
    }

    _tensor *= 1.0f / (float)sampleCount;
}

Cuboid::Cuboid(cmx::Transformable *parent) : Convex{parent}
{
    buildConvex({{-1, -1, -1}, {-1, -1, 1}, {-1, 1, -1}, {-1, 1, 1}, {1, -1, -1}, {1, -1, 1}, {1, 1, -1}, {1, 1, 1}});
}

std::string Cuboid::getName() const
{
    return PRIMITIVE_CUBE;
}

Plane::Plane(cmx::Transformable *parent) : Convex{parent}
{
    buildConvex({{-1, 0, -1}, {-1, 0, 1}, {1, 0, -1}, {1, 0, 1}});
}

std::string Plane::getName() const
{
    return PRIMITIVE_PLANE;
}

bool Convex::SupportPoint::operator==(const SupportPoint &other) const
{
    return glm::length2(xyz - other.xyz) < glm::epsilon<float>();
}
} // namespace cmx
