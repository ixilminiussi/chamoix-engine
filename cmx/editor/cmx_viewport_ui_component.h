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
};

} // namespace cmx
