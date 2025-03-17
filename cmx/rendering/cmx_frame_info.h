#ifndef CMX_FRAME_INFO
#define CMX_FRAME_INFO

// lib
#include <vulkan/vulkan.hpp>

namespace cmx
{

struct FrameInfo
{
    int frameIndex;
    vk::CommandBuffer &commandBuffer;
    vk::DescriptorSet globalDescriptorSet;
};

} // namespace cmx

#endif
