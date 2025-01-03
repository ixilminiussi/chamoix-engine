#ifndef CMX_POINT_LIGHT_COMPONENT
#define CMX_POINT_LIGHT_COMPONENT

#include "cmx_component.h"

// lib
#include <vulkan/vulkan_core.h>

namespace cmx
{

class PointLightComponent : public Component
{
  public:
    PointLightComponent();
    ~PointLightComponent() = default;

    void onAttach() override;
    void onDetach() override;

    void update(float dt) override;
    void render(class FrameInfo &, VkPipelineLayout) override;

    void editor(int i) override;
    void load(tinyxml2::XMLElement *componentElement) override;
    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentComponent) override;

  private:
    float _lightIntensity{1.0f};
    glm::vec3 _lightColor{1.0f};

    glm::vec3 _absolutePosition{};
    glm::vec2 _absoluteScaleXY{1.0f};

    uint32_t _key;
    static uint32_t _keyChain;
};

} // namespace cmx

#endif
