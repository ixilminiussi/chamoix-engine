#ifndef SHAPES
#define SHAPES

// cmx
#include "cmx/cmx_physics_component.h"
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
    float depth{0.f};

    void flip();
};

inline void HitInfo::flip()
{
    normal = -normal;
}

class Shape : public Transformable
{
  public:
    Shape(Transformable *parent);

    virtual void render(const class FrameInfo &, vk::PipelineLayout, class AssetsManager *) {};

    virtual bool overlapsWith(const Shape &, HitInfo &) const = 0;
    virtual bool overlapsWith(const class Sphere &, HitInfo &) const = 0;
    virtual bool overlapsWith(const class Plane &, HitInfo &) const = 0;
    virtual bool overlapsWith(const class Cuboid &, HitInfo &) const = 0;

    Transform getAbsoluteTransform() const override;
    const Transform &getRelativeTransform() const override;

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
    Sphere(Transformable *);
    ~Sphere() {};

    virtual void render(const class FrameInfo &, vk::PipelineLayout, class AssetsManager *) override;

    virtual bool overlapsWith(const Shape &, HitInfo &) const override;
    virtual bool overlapsWith(const Sphere &, HitInfo &) const override;
    virtual bool overlapsWith(const Plane &, HitInfo &) const override;
    virtual bool overlapsWith(const Cuboid &, HitInfo &) const override;

    virtual std::string getName() const override;

    glm::vec3 getCenter() const;
    float getRadius() const;
};

struct Plane : public Shape
{
    Plane(Transformable *);

    virtual void render(const class FrameInfo &, vk::PipelineLayout, class AssetsManager *) override;

    ~Plane() {};

    virtual bool overlapsWith(const Shape &, HitInfo &) const override;
    virtual bool overlapsWith(const Sphere &, HitInfo &) const override;
    virtual bool overlapsWith(const Plane &, HitInfo &) const override;
    virtual bool overlapsWith(const Cuboid &, HitInfo &) const override;

    virtual std::string getName() const override;

    glm::vec4 getMin(const glm::mat4 & = glm::mat4{1.f}) const;
    glm::vec4 getMax(const glm::mat4 & = glm::mat4{1.f}) const;
};

class Cuboid : public Shape
{
  public:
    Cuboid(Transformable *);

    virtual void render(const class FrameInfo &, vk::PipelineLayout, class AssetsManager *) override;

    ~Cuboid() {};

    virtual bool overlapsWith(const Shape &, HitInfo &) const override;
    virtual bool overlapsWith(const Sphere &, HitInfo &) const override;
    virtual bool overlapsWith(const Plane &, HitInfo &) const override;
    virtual bool overlapsWith(const Cuboid &, HitInfo &) const override;

    virtual std::string getName() const override;

    glm::vec4 getMin(const glm::mat4 & = glm::mat4{1.f}) const;
    glm::vec4 getMax(const glm::mat4 & = glm::mat4{1.f}) const;
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
