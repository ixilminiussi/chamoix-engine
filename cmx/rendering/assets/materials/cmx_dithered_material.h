#ifndef CMX_DITHERED_MATERIAL
#define CMX_DITHERED_MATERIAL

// cmx
#include "cmx_material.h"
#include "cmx_register.h"

// lib
#include <glm/ext/matrix_float4x4.hpp>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace cmx
{

#ifndef SIMPLE_PUSH_CONSTANT_DATA
#define SIMPLE_PUSH_CONSTANT_DATA
struct SimplePushConstantData
{
    glm::mat4 modelMatrix{1.f};
    glm::mat4 normalMatrix{1.f};
};
#endif

class DitheredMaterial : public Material
{
  public:
    DitheredMaterial() : Material{"shaders/dithered.vert.spv", "shaders/dithered.frag.spv"} {};

    CLONEABLE_MATERIAL(DitheredMaterial)

    void bind(struct FrameInfo *, const class Drawable *) override;
    void editor() override;
    tinyxml2::XMLElement *save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const override;
    void load(tinyxml2::XMLElement *materialElement) override;

    void initialize() override;

  protected:
    void createPipelineLayout(std::vector<vk::DescriptorSetLayout>) override;
    void createPipeline(vk::RenderPass) override;

    glm::vec3 _color{1.f, 1.f, 1.f};
    glm::vec2 _UVoffset{};
    bool _worldSpaceUV{false};
    float _UVScale{1.f};
    float _UVRotate{0.f};
    bool _textured{true};
};

} // namespace cmx

REGISTER_MATERIAL(cmx::DitheredMaterial)

#endif
