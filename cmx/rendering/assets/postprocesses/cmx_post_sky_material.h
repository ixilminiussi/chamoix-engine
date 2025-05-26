#ifndef CMX_POST_SKY_MATERIAL
#define CMX_POST_SKY_MATERIAL

// cmx
#include "cmx_material.h"
#include "cmx_register.h"

// lib
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace cmx
{

class PostSkyMaterial : public Material
{
  public:
    PostSkyMaterial() : Material{"postprocess.vert.spv", "post_sky.frag.spv", false, ePostProcess} {};

    CLONEABLE_MATERIAL(PostSkyMaterial)

    void bind(struct FrameInfo *, const class Drawable *) override;
    void editor() override;
    tinyxml2::XMLElement *save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const override;
    void load(tinyxml2::XMLElement *materialElement) override;

    void initialize() override;

  protected:
    void createPipelineLayout(std::vector<vk::DescriptorSetLayout>) override;
    void createPipeline(vk::RenderPass) override;
};

} // namespace cmx

REGISTER_MATERIAL(cmx::PostSkyMaterial)

#endif
