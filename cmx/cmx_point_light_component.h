#ifndef CMX_POINT_LIGHT_COMPONENT
#define CMX_POINT_LIGHT_COMPONENT

#include "cmx_component.h"
#include "cmx_register.h"

// lib
#include <vulkan/vulkan_core.h>

namespace cmx
{

class PointLightComponent : public Component
{
  public:
    PointLightComponent();
    ~PointLightComponent() = default;

    CLONEABLE(PointLightComponent)

    void onAttach() override;
    void onDetach() override;

    void render(const class FrameInfo &, vk::PipelineLayout) override;

    void editor(int i) override;
    void load(tinyxml2::XMLElement *componentElement) override;
    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentComponent) override;

    float getLightIntensity() const
    {
        return _lightIntensity;
    }
    void setLightIntensity(float lightIntensity)
    {
        _lightIntensity = lightIntensity;
    }

    const glm::vec3 &getLightColor()
    {
        return _lightColor;
    }
    void setLightColor(const glm::vec3 &lightColor)
    {
        _lightColor = lightColor;
    }

  private:
    class Texture *_texture{nullptr};

    float _lightIntensity{1.0f};
    glm::vec3 _lightColor{1.0f};

    glm::vec3 _absolutePosition{};
    glm::vec2 _absoluteScaleXY{1.0f};

    uint32_t _key;
    static uint32_t _keyChain;
};

REGISTER_COMPONENT(cmx::PointLightComponent)

} // namespace cmx

#endif
