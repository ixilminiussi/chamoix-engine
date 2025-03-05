#ifndef CMX_BILLBOARD_MATERIAL
#define CMX_BILLBOARD_MATERIAL

// cmx
#include "cmx_material.h"
#include "cmx_register.h"

// lib
#include <glm/ext/matrix_float4x4.hpp>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace cmx
{

struct BillboardPushConstant
{
    glm::vec4 position;
    glm::vec4 color;
    glm::vec2 scale;
};

class BillboardMaterial : public Material
{
  public:
    BillboardMaterial() : Material{"shaders/billboard.vert.spv", "shaders/billboard.frag.spv", false} {};

    CLONEABLE_MATERIAL(BillboardMaterial)

    void bind(struct FrameInfo *, const class Drawable *) override;
    void editor() override;
    tinyxml2::XMLElement *save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const override;
    void load(tinyxml2::XMLElement *materialElement) override;

    void initialize() override;

    void setHue(const glm::vec4 hue)
    {
        _hue = hue;
    }

  protected:
    void createPipelineLayout(std::vector<vk::DescriptorSetLayout>) override;
    void createPipeline(vk::RenderPass) override;

    glm::vec4 _hue{1.f};
};

} // namespace cmx

REGISTER_MATERIAL(cmx::BillboardMaterial)

#endif
