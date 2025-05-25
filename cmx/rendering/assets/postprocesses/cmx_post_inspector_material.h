#ifndef CMX_POST_INSPECTOR_MATERIAL
#define CMX_POST_INSPECTOR_MATERIAL

// cmx
#include "cmx_material.h"
#include "cmx_register.h"

// lib
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace cmx
{

class PostInspectorMaterial : public Material
{
  public:
    PostInspectorMaterial() : Material{"postprocess.vert.spv", "post_inspector.frag.spv", false, ePostProcess} {};

    CLONEABLE_MATERIAL(PostInspectorMaterial)

    void bind(struct FrameInfo *, const class Drawable *) override;
    void editor() override;
    tinyxml2::XMLElement *save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const override;
    void load(tinyxml2::XMLElement *materialElement) override;

    void initialize() override;

  protected:
    struct PushConstantData
    {
        int status;
        float nearPlane, farPlane;
    };

    void createPipelineLayout(std::vector<vk::DescriptorSetLayout>) override;
    void createPipeline(vk::RenderPass) override;

    int _status; // 0 for color, 1 for normals, 2 for depth
};

} // namespace cmx

REGISTER_MATERIAL(cmx::PostInspectorMaterial)

#endif
