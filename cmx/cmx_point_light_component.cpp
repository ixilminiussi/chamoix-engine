#include "cmx_point_light_component.h"

// cmx
#include "cmx_actor.h"
#include "cmx_frame_info.h"
#include "cmx_render_system.h"

// lib
#include <vulkan/vulkan_core.h>

namespace cmx
{

PointLightComponent::PointLightComponent()
{
    _renderZ = 0;
    _requestedRenderSystem = BILLBOARD_RENDER_SYSTEM;
}

void PointLightComponent::onAttach()
{
}

void PointLightComponent::render(FrameInfo &frameInfo, VkPipelineLayout pipelineLayout)
{
    if (getParent() == nullptr)
    {
        spdlog::critical("MeshComponent: _parent is expired");
        return;
    }

    vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
}

} // namespace cmx
