#ifndef CMX_FRAME_INFO
#define CMX_FRAME_INFO

// lib
#include <vulkan/vulkan.h>

namespace cmx
{

struct FrameInfo
{
    int frameIndex;
    VkCommandBuffer commandBuffer;
    class Camera &camera;
    VkDescriptorSet globalDescriptorSet;
};

} // namespace cmx

#endif
