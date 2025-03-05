#ifndef CMX_MESH_MATERIAL
#define CMX_MESH_MATERIAL

// cmx
#include "cmx_material.h"
#include "cmx_register.h"

// lib
#include <glm/ext/matrix_float4x4.hpp>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace cmx
{

struct EdgePushConstantData
{
    glm::mat4 modelMatrix{1.f};
    glm::vec3 color;
};

class MeshMaterial : public Material
{
  public:
    MeshMaterial() : Material{"shaders/mesh.vert.spv", "shaders/mesh.frag.spv"} {};

    CLONEABLE_MATERIAL(MeshMaterial)

    void bind(struct FrameInfo *, const class Drawable *) override;
    void editor() override;
    tinyxml2::XMLElement *save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const override;
    void load(tinyxml2::XMLElement *materialElement) override;

    void initialize() override;

    void setColor(const glm::vec3 color)
    {
        _color = color;
    }

  protected:
    void createPipelineLayout(std::vector<vk::DescriptorSetLayout>) override;
    void createPipeline(vk::RenderPass) override;

    glm::vec3 _color{0.f, 0.f, 1.f};
};

} // namespace cmx

REGISTER_MATERIAL(cmx::MeshMaterial)

#endif
