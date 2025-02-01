#ifndef CMX_MESH_COMPONENT
#define CMX_MESH_COMPONENT

#include "cmx_component.h"
#include "tinyxml2.h"

// lib
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

    void onAttach() override;

    void render(const class FrameInfo &, vk::PipelineLayout) override;

    void setModel(const std::string &name);
    std::string getModelName() const;

    void setTexture(const std::string &name);
    std::string getTextureName() const;

    void setColor(const glm::vec3 &color);

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &, tinyxml2::XMLElement *) override;
    void load(tinyxml2::XMLElement *) override;
    void editor(int i) override;

  private:
    class Model *_model{nullptr};
    class Texture *_texture{nullptr};
    glm::vec3 _color{1.f};
};

} // namespace cmx

#endif
