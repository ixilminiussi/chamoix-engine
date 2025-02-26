#ifndef CMX_MESH_COMPONENT
#define CMX_MESH_COMPONENT

#include "cmx_component.h"

// cmx
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

class MeshComponent : public Component
{
  public:
    MeshComponent();
    ~MeshComponent() = default;

    CLONEABLE(MeshComponent)

    void onAttach() override;

    void render(const struct FrameInfo &, vk::PipelineLayout) override;

    void setModel(const std::string &name);
    std::string getModelName() const;

    void setTexture(const std::string &name);
    std::string getTextureName() const;

    bool isTextured() const
    {
        return _textured;
    }
    void setTextured(bool textured)
    {
        _textured = textured;
    }

    void setColor(const glm::vec3 &color);

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &, tinyxml2::XMLElement *) const override;
    void load(tinyxml2::XMLElement *) override;
    void editor(int i) override;

  private:
    class Model *_model{nullptr};
    class Texture *_texture{nullptr};
    bool _worldSpaceUV{false};
    glm::vec2 _UVOffset{0.f};
    float _UVScale{1.f};
    float _UVRotate{0.f};
    glm::vec3 _color{1.f};
    bool _textured{false};
};

} // namespace cmx

REGISTER_COMPONENT(cmx::MeshComponent)

#endif
