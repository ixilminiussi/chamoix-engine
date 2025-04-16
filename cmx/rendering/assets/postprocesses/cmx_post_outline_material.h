#ifndef CMX_POST_OUTLINE_MATERIAL
#define CMX_POST_OUTLINE_MATERIAL

// cmx
#include "cmx_material.h"
#include "cmx_register.h"

// lib
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace cmx
{

class PostOutlineMaterial : public Material
{
  public:
    PostOutlineMaterial() : Material{"shaders/postprocess.vert.spv", "shaders/post_outline.frag.spv", false} {};

    CLONEABLE_MATERIAL(PostOutlineMaterial)

    void bind(struct FrameInfo *, const class Drawable *) override;
    void editor() override;
    tinyxml2::XMLElement *save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const override;
    void load(tinyxml2::XMLElement *materialElement) override;

    void initialize() override;

  protected:
    struct PushConstantData
    {
        glm::vec2 colorEdgeSettings;
        glm::vec2 normalEdgeSettings;
        glm::vec2 depthEdgeSettings;
        float nearPlane, farPlane;
    };

    void createPipelineLayout(std::vector<vk::DescriptorSetLayout>) override;
    void createPipeline(vk::RenderPass) override;

    float _depthEdgeThickness{3.f}, _depthEdgeThreshold{1.5f};
    float _normalEdgeThickness, _normalEdgeThreshold;
    float _colorEdgeThickness, _colorEdgeThreshold;
};

} // namespace cmx

REGISTER_MATERIAL(cmx::PostOutlineMaterial)

#endif
