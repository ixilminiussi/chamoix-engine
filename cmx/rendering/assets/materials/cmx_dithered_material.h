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

class DitheredMaterial : public Material
{
  public:
    DitheredMaterial() : Material{"dithered.vert.spv", "dithered.frag.spv"} {};

    CLONEABLE_MATERIAL(DitheredMaterial)

    void bind(struct FrameInfo *, const class Drawable *) override;
    void editor() override;
    tinyxml2::XMLElement *save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const override;
    void load(tinyxml2::XMLElement *materialElement) override;

    void initialize() override;

  protected:
    void createPipelineLayout(std::vector<vk::DescriptorSetLayout>) override;
    void createPipeline(vk::RenderPass) override;

    glm::vec3 _lightColor{0.596, 0.624, 0.808};
    glm::vec3 _darkColor{0.153, 0.157, 0.22};
    float _scale{5.f};
    float _threshold{1.f};
    bool _lightDots{true};
};

} // namespace cmx

REGISTER_MATERIAL(cmx::DitheredMaterial)

#endif
