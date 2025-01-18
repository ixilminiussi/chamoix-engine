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

    virtual void render(const class FrameInfo &, VkPipelineLayout, class AssetsManager *) {};

    virtual bool overlapsWith(const CmxShape &) const = 0;
    virtual bool overlapsWith(const class CmxCuboid &) const = 0;
    virtual bool overlapsWith(const class CmxContainer &) const = 0;
    virtual bool overlapsWith(const class CmxSphere &) const = 0;

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

    virtual bool overlapsWith(const CmxShape &) const override;
    virtual bool overlapsWith(const CmxSphere &) const override;
    virtual bool overlapsWith(const CmxCuboid &) const override;
    virtual bool overlapsWith(const CmxContainer &) const override;

    virtual std::string getName() const override;

    friend class CmxShape;
    friend class CmxContainer;
    friend class CmxCuboid;

  private:
    glm::vec3 getCenter() const;
    float getRadius() const;
};

struct CmxPolygon
{
    glm::vec3 *a, *b, *c;

    glm::vec3 getNormal();
};

class CmxCuboid : public CmxShape
{
  public:
    CmxCuboid(Transformable *);

    virtual void render(const class FrameInfo &, VkPipelineLayout, class AssetsManager *) override;

    ~CmxCuboid() {};

    virtual bool overlapsWith(const CmxShape &) const override;
    virtual bool overlapsWith(const CmxSphere &) const override;
    virtual bool overlapsWith(const CmxCuboid &) const override;
    virtual bool overlapsWith(const CmxContainer &) const override;

    virtual std::string getName() const override;

    friend class CmxSphere;
    friend class CmxContainer;
    friend class CmxShape;

  protected:
    glm::vec4 getA(const glm::mat4 & = glm::mat4{1.f}) const;
    glm::vec4 getB(const glm::mat4 & = glm::mat4{1.f}) const;
    glm::vec4 getC(const glm::mat4 & = glm::mat4{1.f}) const;
    glm::vec4 getD(const glm::mat4 & = glm::mat4{1.f}) const;
    glm::vec4 getE(const glm::mat4 & = glm::mat4{1.f}) const;
    glm::vec4 getF(const glm::mat4 & = glm::mat4{1.f}) const;
    glm::vec4 getG(const glm::mat4 & = glm::mat4{1.f}) const;
    glm::vec4 getH(const glm::mat4 & = glm::mat4{1.f}) const;
};

class CmxContainer : public CmxCuboid
{
  public:
    CmxContainer(Transformable *);

    virtual void render(const class FrameInfo &, VkPipelineLayout, class AssetsManager *) override;

    ~CmxContainer() {};

    virtual bool overlapsWith(const CmxShape &) const override;
    virtual bool overlapsWith(const CmxSphere &) const override;
    virtual bool overlapsWith(const CmxCuboid &) const override;
    virtual bool overlapsWith(const CmxContainer &) const override;

    virtual std::string getName() const override;

    friend class CmxSphere;
    friend class CmxCuboid;
    friend class CmxShape;
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
