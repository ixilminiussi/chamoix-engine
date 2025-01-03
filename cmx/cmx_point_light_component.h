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

    void render(class FrameInfo &, VkPipelineLayout) override;
};

} // namespace cmx

#endif
