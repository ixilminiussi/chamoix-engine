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

namespace m
{
struct PushConstantData
{
    glm::mat4 modelMatrix{1.f};
    glm::vec3 color;
};
} // namespace m

class MeshMaterial : public Material
{
  public:
    MeshMaterial() : Material{"mesh.vert.spv", "mesh.frag.spv"} {};

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
