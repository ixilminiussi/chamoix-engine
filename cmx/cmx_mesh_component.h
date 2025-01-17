#ifndef CMX_MESH_COMPONENT
#define CMX_MESH_COMPONENT

#include "cmx_component.h"
#include "tinyxml2.h"

// lib
#include <vulkan/vulkan_core.h>

// std
#include <memory>

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

    void render(const class FrameInfo &, VkPipelineLayout) override;
    void setModel(Primitives);
    void setModel(const std::string &name);
    std::string getModelName();

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &, tinyxml2::XMLElement *) override;
    void load(tinyxml2::XMLElement *) override;
    void editor(int i) override;

  private:
    class CmxModel *_cmxModel;
    glm::vec3 _color;
};

} // namespace cmx

#endif
