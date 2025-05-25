#ifndef CMX_POST_BLUR_MATERIAL
#define CMX_POST_BLUR_MATERIAL

// cmx
#include "cmx_material.h"
#include "cmx_register.h"

// lib
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace cmx
{

class PostBlurMaterial : public Material
{
  public:
    PostBlurMaterial() : Material{"postprocess.vert.spv", "post_blur.frag.spv", false, ePostProcess} {};

    CLONEABLE_MATERIAL(PostBlurMaterial)

    void bind(struct FrameInfo *, const class Drawable *) override;
    void editor() override;
    tinyxml2::XMLElement *save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const override;
    void load(tinyxml2::XMLElement *materialElement) override;

    void initialize() override;
    void free() override;

  protected:
    struct PushConstantData
    {
        float nearPlane;
        float farPlane;
    };

    void createPipelineLayout(std::vector<vk::DescriptorSetLayout>) override;
    void createPipeline(vk::RenderPass) override;

    class Texture *noiseTexture{nullptr};
};

} // namespace cmx

REGISTER_MATERIAL(cmx::PostBlurMaterial)

#endif
