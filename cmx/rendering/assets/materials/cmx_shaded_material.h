#ifndef CMX_SHADED_MATERIAL
#define CMX_SHADED_MATERIAL

// cmx
#include "cmx_material.h"

// lib
#include <glm/ext/matrix_float4x4.hpp>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace cmx
{

#ifndef SIMPLE_PUSH_CONSTANT
#define SIMPLE_PUSH_CONSTANT
struct SimplePushConstantData
{
    glm::mat4 modelMatrix{1.f};
    glm::mat4 normalMatrix{1.f};
};
#endif

class ShadedMaterial : public Material
{
  public:
    ShadedMaterial() : Material{"shaders/shaded.vert.spv", "shaders/shaded.frag.spv"} {};

    void bind(struct FrameInfo *) override;
    void editor() override;

    void initialize() override;

  protected:
    void createPipelineLayout(std::vector<vk::DescriptorSetLayout>) override;
    void createPipeline(vk::RenderPass) override;
};

} // namespace cmx

#endif
