#pragma once

#include "cmx_component.h"

namespace cmx
{

class ViewportUIComponent : public Component
{
  public:
    ViewportUIComponent();
    ~ViewportUIComponent() = default;

    void render(VkCommandBuffer, VkPipelineLayout, const class CameraComponent &);

    void initImGUI(class CmxDevice &, class CmxWindow &, class CmxRenderer &);

  private:
    bool initialized{false};
};

} // namespace cmx
