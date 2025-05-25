#ifndef CMX_POST_SSAO_MATERIAL
#define CMX_POST_SSAO_MATERIAL

// cmx
#include "cmx_material.h"
#include "cmx_register.h"

// lib
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace cmx
{

class PostSSAOMaterial : public Material
{
  public:
    PostSSAOMaterial() : Material{"postprocess.vert.spv", "post_ssao.frag.spv", false, ePostProcess} {};

    CLONEABLE_MATERIAL(PostSSAOMaterial)

    void bind(struct FrameInfo *, const class Drawable *) override;
    void editor() override;
    tinyxml2::XMLElement *save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const override;
    void load(tinyxml2::XMLElement *materialElement) override;

    void initialize() override;
    void free() override;

  protected:
    struct PushConstantData
    {
        glm::mat4 projection;
        glm::mat4 view;
    };

    void createPipelineLayout(std::vector<vk::DescriptorSetLayout>) override;
    void createPipeline(vk::RenderPass) override;

    class Texture *noiseTexture{nullptr};
};

} // namespace cmx

REGISTER_MATERIAL(cmx::PostSSAOMaterial)

#endif
