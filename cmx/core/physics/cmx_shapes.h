#ifndef CMX_SHAPES
#define CMX_SHAPES

// cmx
#include "cmx_physics_body.h"
#include "cmx_transform.h"

// lib
#include <glm/ext/scalar_constants.hpp>
#include <glm/ext/vector_float3.hpp>
#include <set>
#include <vulkan/vulkan.hpp>

namespace cmx
{

struct HitInfo
{
    glm::vec3 normal{0.f};
    glm::vec3 point{0.f};
    float depth{0.f};

    void flip();
    [[nodiscard]] HitInfo getFlipped() const;
};

inline void HitInfo::flip()
{
    normal = -normal;
    point += normal * depth;
}

inline HitInfo HitInfo::getFlipped() const
{
    HitInfo copy = *this;
    copy.flip();

    return copy;
}

class Shape : public virtual Transformable
{
  public:
    Shape(Transformable *parent);

    virtual bool overlapsWith(const Shape &, HitInfo &) const = 0;
    virtual bool overlapsWith(const class Sphere &, HitInfo &) const = 0;
    virtual bool overlapsWith(const class Convex &, HitInfo &) const = 0;

    [[nodiscard]] const Transform &getLocalSpaceTransform() const override;
    [[nodiscard]] Transform getWorldSpaceTransform(int depth = -1) const override;
    [[nodiscard]] virtual glm::vec3 getCenterOfMass() const = 0;
    [[nodiscard]] virtual glm::mat3 getInertiaTensor() const = 0;
    bool wasOverlapping(class PhysicsBody *) const;
    bool isOverlapping(class PhysicsBody *) const;
    [[nodiscard]] bool isOverlapping() const;

    [[nodiscard]] virtual glm::vec3 support(const glm::vec3 &dir, const float bias) const = 0;
    [[nodiscard]] virtual class Model *getModel(class AssetsManager *) const = 0;
    [[nodiscard]] virtual std::string getName() const = 0;

    void addOverlappingComponent(class PhysicsBody *);
    void reassess();
    void swapBuffer();

    void setMask(uint8_t mask);

    uint8_t mask = MASK_ALL;

  protected:
    Transformable *_parent;

    uint8_t _buffer{0}, _alternativeBuffer{1};
    std::array<std::set<class PhysicsBody *>, 2> _overlappingComponents;

    glm::vec3 _centerOfMass;
};

class Sphere : public Shape
{
  public:
    Sphere(cmx::Transformable *);

    bool overlapsWith(const Shape &, HitInfo &) const override;
    bool overlapsWith(const Sphere &, HitInfo &) const override;
    bool overlapsWith(const Convex &, HitInfo &) const override;

    [[nodiscard]] glm::vec3 getCenterOfMass() const override;
    [[nodiscard]] glm::mat3 getInertiaTensor() const override;
    [[nodiscard]] class Model *getModel(class AssetsManager *) const override;
    [[nodiscard]] std::string getName() const override;
    [[nodiscard]] glm::vec3 getCenter() const;
    [[nodiscard]] float getRadius() const;

    [[nodiscard]] glm::vec3 support(const glm::vec3 &dir, const float bias) const override;
};

class Convex : public Shape
{
  public:
    Convex(cmx::Transformable *);
    ~Convex() override;

    void buildConvex(const std::vector<glm::vec3> &vertices);
    bool overlapsWith(const Shape &, HitInfo &) const override;
    bool overlapsWith(const Sphere &, HitInfo &) const override;
    bool overlapsWith(const Convex &, HitInfo &) const override;

    [[nodiscard]] glm::vec3 getCenterOfMass() const override;
    [[nodiscard]] glm::mat3 getInertiaTensor() const override;
    [[nodiscard]] class Model *getModel(class AssetsManager *) const override;
    [[nodiscard]] std::string getName() const override;
    [[nodiscard]] std::vector<glm::vec3> getWorldSpaceVertices() const;

    [[nodiscard]] glm::vec3 support(const glm::vec3 &dir, const float bias) const override;

  protected:
    struct Face
    {
        int a, b, c;
    };
    struct Edge
    {
        int a, b;

        bool operator==(const Edge &other) const
        {
            return a == other.a && b == other.b;
        }
    };
    struct SupportPoint
    {
        glm::vec3 xyz;
        glm::vec3 a;
        glm::vec3 b;

        bool operator==(const SupportPoint &other) const;
    };

    [[nodiscard]] std::array<Edge, 3> getEdges(const Face &) const;
    [[nodiscard]] bool isEdgeUnique(const std::vector<const Face *> &face, const Edge &, int ignoreIdx = -1) const;

    void expandConvex(const glm::vec3 &vertex);
    float expandEPA(const Shape &other, const float bias, const std::array<SupportPoint, 4> simplexPoints,
                    glm::vec3 &pointOnA, glm::vec3 &pointOnB) const;
    [[nodiscard]] bool isInternalVertex(const glm::vec3 &vertex) const;
    void cleanInternalVertices();
    [[nodiscard]] int removeFacesFacingPoint(const glm::vec3 &p, std::vector<Face> &,
                                             std::vector<SupportPoint> &) const;
    void findDanglingEdges(std::vector<Edge> &, const std::vector<Face> &) const;

    void calculateCenterOfMass();
    void calculateInertiaTensor();
    void makeModel();

    [[nodiscard]] int getClosestFace(const glm::vec3 &p, const std::vector<Face> &,
                                     const std::vector<SupportPoint> &) const;

    [[nodiscard]] bool simplexHasPoint(const std::array<SupportPoint, 4> &simplex, const int numPoints,
                                       const SupportPoint &point) const;
    void sortValids(std::array<SupportPoint, 4> &, glm::vec4 &lambdas) const;
    [[nodiscard]] bool simplexSignedVolumes(const std::array<SupportPoint, 4> &simplex, const int numPoints,
                                            glm::vec3 &d, glm::vec4 &lambdasOut) const;

    std::vector<glm::vec3> _vertices;
    std::vector<Face> _faceIndices;
    glm::vec3 _centerOfMass;
    glm::mat3 _tensor;

    class Model *_model{nullptr};
};

class Cuboid : public Convex
{
  public:
    Cuboid(cmx::Transformable *);
    [[nodiscard]] std::string getName() const override;
};

class Plane : public Convex
{
  public:
    Plane(cmx::Transformable *);
    [[nodiscard]] std::string getName() const override;
};

} // namespace cmx

#endif
