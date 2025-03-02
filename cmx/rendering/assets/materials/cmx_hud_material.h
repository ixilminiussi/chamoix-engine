#ifndef CMX_HUD_MATERIAL
#define CMX_HUD_MATERIAL

// cmx
#include "cmx_material.h"

// lib
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

// std
#include <memory>

namespace cmx
{

class HudMaterial : public Material
{
  public:
    void bind(struct FrameInfo *) override;
    void editor() override;

    void initialize() override;

  protected:
    HudMaterial() : Material{"shaders/hud.vert.spv", "shaders/hud.frag.spv"} {};

    void createPipelineLayout(std::vector<vk::DescriptorSetLayout>) override;
    void createPipeline(vk::RenderPass) override;
};

} // namespace cmx

#endif
