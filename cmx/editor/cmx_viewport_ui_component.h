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

  private:
    class CmxWindow *cmxWindow;
    bool initialized{false};
};

} // namespace cmx
