#ifndef CMX_SHADED_MATERIAL
#define CMX_SHADED_MATERIAL

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

struct SimplePushConstantData
{
    glm::mat4 modelMatrix{1.f};
    glm::mat4 normalMatrix{1.f};
};

class ShadedMaterial : public Material
{
  public:
    void bind(struct FrameInfo *) override;
    void editor() override;

    void initialize() override;

  protected:
    ShadedMaterial() : Material{"shaders/shaded.vert.spv", "shaders/shaded.frag.spv"} {};

    void createPipelineLayout(std::vector<vk::DescriptorSetLayout>) override;
    void createPipeline(vk::RenderPass) override;
};

} // namespace cmx

#endif
