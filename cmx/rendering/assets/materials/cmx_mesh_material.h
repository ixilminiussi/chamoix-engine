#ifndef CMX_MESH_MATERIAL
#define CMX_MESH_MATERIAL

// cmx
#include "cmx_material.h"

// lib
#include <glm/ext/matrix_float4x4.hpp>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

// std
#include <memory>

namespace cmx
{

class MeshMaterial : public Material
{
  public:
    MeshMaterial() : Material{"shaders/mesh.vert.spv", "shaders/mesh.frag.spv"} {};

    void bind(struct FrameInfo *) override;
    void editor() override;

    void initialize() override;

  protected:
    void createPipelineLayout(std::vector<vk::DescriptorSetLayout>) override;
    void createPipeline(vk::RenderPass) override;
};

} // namespace cmx

#endif
