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
    PostOutlineMaterial()
        : Material{"shaders/postprocess.vert.spv", "shaders/post_outline.frag.spv", false, ePostProcess} {};

    CLONEABLE_MATERIAL(PostOutlineMaterial)

    void bind(struct FrameInfo *, const class Drawable *) override;
    void editor() override;
    tinyxml2::XMLElement *save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const override;
    void load(tinyxml2::XMLElement *materialElement) override;

    void initialize() override;

  protected:
    struct PushConstantData
    {
        glm::vec3 edgeColor;
        float colorEdgeThickness, colorEdgeThreshold, colorDepthFactor;
        float normalEdgeThickness, normalEdgeThreshold, normalDepthFactor;
        float depthEdgeThickness, depthEdgeThreshold, depthDepthFactor;
        float nearPlane, farPlane;
    };

    void createPipelineLayout(std::vector<vk::DescriptorSetLayout>) override;
    void createPipeline(vk::RenderPass) override;

    glm::vec3 _edgeColor{1.f};
    float _depthEdgeThickness{3.f}, _depthEdgeThreshold{0.5f}, _depthDepthFactor{0.51f};
    float _normalEdgeThickness{3.f}, _normalEdgeThreshold{0.04f}, _normalDepthFactor{0.04f};
    float _colorEdgeThickness, _colorEdgeThreshold, _colorDepthFactor;
};

} // namespace cmx

REGISTER_MATERIAL(cmx::PostOutlineMaterial)

#endif
