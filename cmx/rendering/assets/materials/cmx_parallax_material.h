#ifndef CMX_PARALLAX_MATERIAL
#define CMX_PARALLAX_MATERIAL

// cmx
#include "cmx_material.h"
#include "cmx_register.h"

// lib
#include <glm/ext/matrix_float4x4.hpp>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace cmx
{

class ParallaxMaterial : public Material
{
  public:
    ParallaxMaterial() : Material{"shaders/parallax.vert.spv", "shaders/parallax.frag.spv"} {};

    CLONEABLE_MATERIAL(ParallaxMaterial)

    void bind(struct FrameInfo *, const class Drawable *) override;
    void editor() override;
    tinyxml2::XMLElement *save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const override;
    void load(tinyxml2::XMLElement *materialElement) override;

    void initialize() override;

  protected:
    void createPipelineLayout(std::vector<vk::DescriptorSetLayout>) override;
    void createPipeline(vk::RenderPass) override;

    glm::vec2 _UVoffset{};
    bool _worldSpaceUV{false};
    float _UVScale{1.f};
    float _UVRotate{0.f};
    float _parallaxDepth{0.2f};
};

} // namespace cmx

REGISTER_MATERIAL(cmx::ParallaxMaterial)

#endif
