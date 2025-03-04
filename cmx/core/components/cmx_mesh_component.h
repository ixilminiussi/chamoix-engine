#ifndef CMX_MESH_COMPONENT
#define CMX_MESH_COMPONENT

#include "cmx_component.h"

// cmx
#include "cmx_drawable.h"
#include "cmx_register.h"

// lib
#include "tinyxml2.h"
#include <vulkan/vulkan_core.h>

namespace cmx
{

enum Primitives
{
    SPHERE,
    BOX,
    // PLANE,
    // TORUS,
    // CYLINDER,
    // CONE,
    // CAPSULE,
};

class MeshComponent : public Component, public virtual Drawable
{
  public:
    MeshComponent();
    ~MeshComponent() = default;

    CLONEABLE_COMPONENT(MeshComponent)

    void onAttach() override;

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &, tinyxml2::XMLElement *) const override;
    void load(tinyxml2::XMLElement *) override;
    void editor(int i) override;
};

} // namespace cmx

REGISTER_COMPONENT(cmx::MeshComponent)

#endif
