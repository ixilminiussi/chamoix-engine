#ifndef CMX_POST_PASSTHROUGH_MATERIAL
#define CMX_POST_PASSTHROUGH_MATERIAL

// cmx
#include "cmx_material.h"
#include "cmx_register.h"

// lib
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace cmx
{

class PostPassthroughMaterial : public Material
{
  public:
    PostPassthroughMaterial() : Material{"shaders/postprocess.vert.spv", "shaders/post_passthrough.frag.spv", false} {};

    CLONEABLE_MATERIAL(PostPassthroughMaterial)

    void bind(struct FrameInfo *, const class Drawable *) override;
    void editor() override;
    tinyxml2::XMLElement *save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const override;
    void load(tinyxml2::XMLElement *materialElement) override;

    void initialize() override;

  protected:
    struct PushConstantData
    {
        int status;
    };

    void createPipelineLayout(std::vector<vk::DescriptorSetLayout>) override;
    void createPipeline(vk::RenderPass) override;

    int _status; // 0 for color, 1 for normals, 2 for depth
};

} // namespace cmx

REGISTER_MATERIAL(cmx::PostPassthroughMaterial)

#endif
