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

    CLONEABLE(MeshComponent)

    void onAttach() override;

    // void render(const struct FrameInfo &, vk::PipelineLayout) override;

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &, tinyxml2::XMLElement *) const override;
    void load(tinyxml2::XMLElement *) override;
    void editor(int i) override;

  private:
    // bool _worldSpaceUV{false};
    // glm::vec2 _UVOffset{0.f};
    // float _UVScale{1.f};
    // float _UVRotate{0.f};
    // glm::vec3 _color{1.f};
    // bool _textured{false};
};

} // namespace cmx

REGISTER_COMPONENT(cmx::MeshComponent)

#endif
