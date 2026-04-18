#ifndef CMX_POST_COMPOSITE_MATERIAL
#define CMX_POST_COMPOSITE_MATERIAL

// cmx
#include "cmx_material.h"
#include "cmx_register.h"

// lib
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace cmx
{

class PostCompositeMaterial : public Material
{
  public:
    PostCompositeMaterial() : Material{"postprocess.vert.spv", "post_composite.frag.spv", false, ePostProcess} {};

    CLONEABLE_MATERIAL(PostCompositeMaterial)

    void bind(struct FrameInfo *, const class Drawable *) override;
    void editor() override;
    tinyxml2::XMLElement *save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const override;
    void load(tinyxml2::XMLElement *materialElement) override;

    void initialize() override;

  protected:
    struct PushConstantData
    {
        glm::vec3 ambientLight{0.5};
        bool useSSAO{false};
    };

    void createPipelineLayout(std::vector<vk::DescriptorSetLayout>) override;
    void createPipeline(vk::RenderPass) override;

    bool _ssaoToggle;
};

} // namespace cmx

REGISTER_MATERIAL(cmx::PostCompositeMaterial)

#endif
