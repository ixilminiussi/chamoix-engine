#pragma once

// lib
#include <vulkan/vulkan.h>

namespace cmx
{

struct FrameInfo
{
    int frameIndex;
    VkCommandBuffer commandBuffer;
    class CameraComponent &camera;
    VkDescriptorSet globalDescriptorSet;
};

} // namespace cmx
