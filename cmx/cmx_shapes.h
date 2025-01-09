#ifndef SHAPES
#define SHAPES

// cmx
#include "cmx_transform.h"

// lib
#include <glm/ext/vector_float3.hpp>

namespace cmx
{

class CmxShape
{
  public:
    CmxShape(const Transformable *parent) : _parent{parent} {};
    virtual bool overlapsWith(const class CmxCuboid &) = 0;
    virtual bool overlapsWith(const class CmxSphere &) = 0;
    virtual void expandToInclude(const glm::vec3 &) {};

    virtual void updateDimensions() = 0;

  protected:
    const Transformable *_parent;
};

class CmxSphere : public CmxShape
{
  public:
    CmxSphere(const glm::vec3 &center, float radius, const Transformable *);
    ~CmxSphere() {};

    bool overlapsWith(const CmxSphere &) override;
    bool overlapsWith(const CmxCuboid &) override;
    void expandToInclude(const glm::vec3 &) override;

    void updateDimensions() override;

  private:
    float _relRadius;
    glm::vec4 _relCenter;

    float _radius;
    glm::vec3 _center;
};

struct CmxPolygon
{
    glm::vec3 *a, *b, *c;

    glm::vec3 getNormal();
};

class CmxCuboid : public CmxShape
{
  public:
    CmxCuboid(const glm::vec3 &cornerA, const glm::vec3 &cornerG, const Transformable *);
    ~CmxCuboid() {};

    bool overlapsWith(const CmxSphere &) override;
    bool overlapsWith(const CmxCuboid &) override;
    void expandToInclude(const glm::vec3 &) override;

    void updateDimensions() override;

  private:
    glm::vec4 _relA, _relB, _relC, _relD, _relE, _relF, _relG, _relH;

    glm::vec3 _a, _b, _c, _d, _e, _f, _g, _h;
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
