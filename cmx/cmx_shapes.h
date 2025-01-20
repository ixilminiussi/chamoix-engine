#ifndef SHAPES
#define SHAPES

// cmx
#include "cmx/cmx_physics_component.h"
#include "cmx_transform.h"

// lib
#include <glm/ext/vector_float3.hpp>
#include <set>
#include <vulkan/vulkan_core.h>

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

class CmxShape : public Transformable
{
  public:
    CmxShape(Transformable *parent);

    virtual void render(const class FrameInfo &, VkPipelineLayout, class AssetsManager *) {};

    virtual bool overlapsWith(const CmxShape &, HitInfo &) const = 0;
    virtual bool overlapsWith(const class CmxSphere &, HitInfo &) const = 0;
    virtual bool overlapsWith(const class CmxPlane &, HitInfo &) const = 0;
    virtual bool overlapsWith(const class CmxCuboid &, HitInfo &) const = 0;

    Transform getAbsoluteTransform() const override;
    const Transform &getRelativeTransform() const override;

    bool wasOverlapping(class PhysicsComponent *) const;
    bool isOverlapping(class PhysicsComponent *) const;
    bool isOverlapping() const;

    virtual std::string getName() const = 0;

    void addOverlappingComponent(class PhysicsComponent *);
    void reassess();
    void swapBuffer();

  protected:
    Transformable *_parent;

    uint8_t _buffer{0}, _alternativeBuffer{1};
    std::set<class PhysicsComponent *> _overlappingComponents[2];
};

class CmxSphere : public CmxShape
{
  public:
    CmxSphere(Transformable *);
    ~CmxSphere() {};

    virtual void render(const class FrameInfo &, VkPipelineLayout, class AssetsManager *) override;

    virtual bool overlapsWith(const CmxShape &, HitInfo &) const override;
    virtual bool overlapsWith(const CmxSphere &, HitInfo &) const override;
    virtual bool overlapsWith(const CmxPlane &, HitInfo &) const override;
    virtual bool overlapsWith(const CmxCuboid &, HitInfo &) const override;

    virtual std::string getName() const override;

    glm::vec3 getCenter() const;
    float getRadius() const;
};

struct CmxPlane : public CmxShape
{
    CmxPlane(Transformable *);

    virtual void render(const class FrameInfo &, VkPipelineLayout, class AssetsManager *) override;

    ~CmxPlane() {};

    virtual bool overlapsWith(const CmxShape &, HitInfo &) const override;
    virtual bool overlapsWith(const CmxSphere &, HitInfo &) const override;
    virtual bool overlapsWith(const CmxPlane &, HitInfo &) const override;
    virtual bool overlapsWith(const CmxCuboid &, HitInfo &) const override;

    virtual std::string getName() const override;

    glm::vec4 getMin(const glm::mat4 & = glm::mat4{1.f}) const;
    glm::vec4 getMax(const glm::mat4 & = glm::mat4{1.f}) const;
};

class CmxCuboid : public CmxShape
{
  public:
    CmxCuboid(Transformable *);

    virtual void render(const class FrameInfo &, VkPipelineLayout, class AssetsManager *) override;

    ~CmxCuboid() {};

    virtual bool overlapsWith(const CmxShape &, HitInfo &) const override;
    virtual bool overlapsWith(const CmxSphere &, HitInfo &) const override;
    virtual bool overlapsWith(const CmxPlane &, HitInfo &) const override;
    virtual bool overlapsWith(const CmxCuboid &, HitInfo &) const override;

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
