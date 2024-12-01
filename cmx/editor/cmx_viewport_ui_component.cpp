#include "cmx_viewport_ui_component.h"

// lib
#include <imgui.h>

// std
#include <limits>

namespace cmx
{

ViewportUIComponent::ViewportUIComponent()
{
    renderZ = std::numeric_limits<int32_t>::max(); // ensures it gets rendered at the very top
}

void ViewportUIComponent::render(VkCommandBuffer, VkPipelineLayout, const class CameraComponent &)
{
}

} // namespace cmx
