#ifndef SHAPES
#define SHAPES

// cmx
#include "cmx_physics_component.h"
#include "cmx_transform.h"

// lib
#include <glm/ext/vector_float3.hpp>
#include <set>
#include <vulkan/vulkan.hpp>

// std
#include <memory>

namespace cmx
{

struct HitInfo
{
    glm::vec3 normal{0.f};
    glm::vec3 point{0.f};
    float depth{0.f};

    void flip();
    HitInfo getFlipped() const;
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

class Shape : private Transformable
{
  public:
    Shape(Transformable *parent);

    virtual void render(const struct FrameInfo &, vk::PipelineLayout, class AssetsManager *) {};

    virtual bool overlapsWith(const Shape &, HitInfo &) const = 0;
    virtual bool overlapsWith(const class Sphere &, HitInfo &) const = 0;
    virtual bool overlapsWith(const class Plane &, HitInfo &) const = 0;
    virtual bool overlapsWith(const class Cuboid &, HitInfo &) const = 0;

    Transform getWorldSpaceTransform() const override;
    const Transform &getLocalSpaceTransform() const override;

    virtual glm::vec3 getCenterOfMass() const = 0;
    virtual glm::mat3 getInertiaTensor() const = 0;

    bool wasOverlapping(class PhysicsComponent *) const;
    bool isOverlapping(class PhysicsComponent *) const;
    bool isOverlapping() const;

    virtual std::string getName() const = 0;

    void addOverlappingComponent(class PhysicsComponent *);
    void reassess();
    void swapBuffer();

    void setMask(uint8_t mask)
    {
        this->mask = mask;
    }

    uint8_t mask = MASK_ALL;

  protected:
    Transformable *_parent;

    uint8_t _buffer{0}, _alternativeBuffer{1};
    std::set<class PhysicsComponent *> _overlappingComponents[2];
};

class Sphere : public Shape
{
  public:
    Sphere(cmx::Transformable *);
    ~Sphere() {};

    void render(const struct FrameInfo &, vk::PipelineLayout, class AssetsManager *) override;

    bool overlapsWith(const Shape &, HitInfo &) const override;
    bool overlapsWith(const Sphere &, HitInfo &) const override;
    bool overlapsWith(const Plane &, HitInfo &) const override;
    bool overlapsWith(const Cuboid &, HitInfo &) const override;

    glm::vec3 getCenterOfMass() const override
    {
        return glm::vec3{0.f};
    }
    glm::mat3 getInertiaTensor() const override;

    virtual std::string getName() const override;

    glm::vec3 getCenter() const;
    float getRadius() const;
};

class Cuboid : public Shape
{
  public:
    Cuboid(cmx::Transformable *);

    virtual void render(const struct FrameInfo &, vk::PipelineLayout, class AssetsManager *) override;

    ~Cuboid() {};

    glm::vec3 getCenterOfMass() const override
    {
        return glm::vec3{0.f};
    }

    virtual bool overlapsWith(const Shape &, HitInfo &) const override;
    virtual bool overlapsWith(const Sphere &, HitInfo &) const override;
    virtual bool overlapsWith(const Plane &, HitInfo &) const override;
    virtual bool overlapsWith(const Cuboid &, HitInfo &) const override;

    virtual std::pair<float, float> projectOnto(const glm::vec3 &) const;
    glm::vec3 getSupportPoint(const glm::vec3 &direction) const;

    virtual glm::mat3 getInertiaTensor() const override;

    virtual std::string getName() const override;

    virtual std::vector<glm::vec3> getVerticesWorldSpace() const;

    virtual glm::vec4 getMinLocalSpace() const;
    glm::vec4 getMinWorldSpace() const;
    virtual glm::vec4 getMaxLocalSpace() const;
    glm::vec4 getMaxWorldSpace() const;
};

struct Plane : public Cuboid
{
    using Cuboid::overlapsWith;

    Plane(cmx::Transformable *);

    virtual void render(const struct FrameInfo &, vk::PipelineLayout, class AssetsManager *) override;

    ~Plane() {};

    glm::vec3 getCenterOfMass() const override
    {
        return glm::vec3{0.f};
    }

    virtual std::pair<float, float> projectOnto(const glm::vec3 &) const override;

    bool overlapsWith(const Plane &, HitInfo &) const override;
    bool overlapsWith(const Cuboid &, HitInfo &) const override;

    virtual std::string getName() const override;

    virtual std::vector<glm::vec3> getVerticesWorldSpace() const override;

    glm::vec4 getMinLocalSpace() const override;
    glm::vec4 getMaxLocalSpace() const override;
};

// A---------B
// |\        |\
// | \       | \
// |  D---------C
// E--|------F  |  Y
//  \ |       \ |  | Z
//   \|        \|  |/
//    H---------G  !----X

} // namespace cmx

#endif
