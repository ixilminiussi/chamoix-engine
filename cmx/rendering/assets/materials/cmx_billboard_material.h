#ifndef CMX_BILLBOARD_MATERIAL
#define CMX_BILLBOARD_MATERIAL

// cmx
#include "cmx_material.h"

// lib
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace cmx
{

class BillboardMaterial : public Material
{
  public:
    BillboardMaterial() : Material{"shaders/billboard.vert.spv", "shaders/billboard.frag.spv"} {};

    void bind(struct FrameInfo *) override;
    void editor() override;

    void initialize() override;

  protected:
    void createPipelineLayout(std::vector<vk::DescriptorSetLayout>) override;
    void createPipeline(vk::RenderPass) override;
};

} // namespace cmx

#endif
