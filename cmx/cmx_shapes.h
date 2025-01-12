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

class CmxShape
{
  public:
    CmxShape(Transformable *parent);

    virtual void render(const class FrameInfo &, VkPipelineLayout, std::shared_ptr<class AssetsManager>) {};

    virtual bool overlapsWith(const CmxShape &) const = 0;
    virtual bool overlapsWith(const class CmxCuboid &) const = 0;
    virtual bool overlapsWith(const class CmxSphere &) const = 0;
    virtual void expandToInclude(const glm::vec3 &) {};

    virtual void updateDimensions() = 0;

    bool wasOverlapping(class PhysicsComponent *) const;
    bool isOverlapping(class PhysicsComponent *) const;
    bool isOverlapping() const;

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
    CmxSphere(const glm::vec3 &center, float radius, Transformable *);
    ~CmxSphere() {};

    virtual void render(const class FrameInfo &, VkPipelineLayout, std::shared_ptr<class AssetsManager>) override;

    bool overlapsWith(const CmxShape &) const override;
    bool overlapsWith(const CmxSphere &) const override;
    bool overlapsWith(const CmxCuboid &) const override;
    void expandToInclude(const glm::vec3 &) override;

    void updateDimensions() override;

    friend class CmxShape;
    friend class CmxCuboid;

  protected:
    float _relRadius;
    glm::vec4 _relCenter;

    float _absRadius;
    glm::vec4 _absCenter;
};

struct CmxPolygon
{
    glm::vec3 *a, *b, *c;

    glm::vec3 getNormal();
};

class CmxCuboid : public CmxShape
{
  public:
    CmxCuboid(const glm::vec3 &cornerA, const glm::vec3 &cornerG, Transformable *);

    virtual void render(const class FrameInfo &, VkPipelineLayout, std::shared_ptr<class AssetsManager>) override;

    ~CmxCuboid() {};

    bool overlapsWith(const CmxShape &) const override;
    bool overlapsWith(const CmxSphere &) const override;
    bool overlapsWith(const CmxCuboid &) const override;
    void expandToInclude(const glm::vec3 &) override;

    void updateDimensions() override;

    friend class CmxSphere;
    friend class CmxShape;

  protected:
    glm::vec4 _relA, _relB, _relC, _relD, _relE, _relF, _relG, _relH;

    glm::vec4 _absA, _absB, _absC, _absD, _absE, _absF, _absG, _absH;
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
