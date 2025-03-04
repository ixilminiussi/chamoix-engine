#ifndef CMX_HUD_MATERIAL
#define CMX_HUD_MATERIAL

// cmx
#include "cmx_material.h"
#include "cmx_register.h"

// lib
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace cmx
{

class HudMaterial : public Material
{
  public:
    HudMaterial() : Material{"shaders/hud.vert.spv", "shaders/hud.frag.spv", false} {};

    void bind(struct FrameInfo *, const class Drawable *) override;
    void editor() override;

    void initialize() override;

  protected:
    void createPipelineLayout(std::vector<vk::DescriptorSetLayout>) override;
    void createPipeline(vk::RenderPass) override;
};

} // namespace cmx

REGISTER_MATERIAL(cmx::HudMaterial)

#endif
