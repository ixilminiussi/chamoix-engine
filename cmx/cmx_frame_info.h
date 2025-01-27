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
    class Camera &camera;
    vk::DescriptorSet globalDescriptorSet;
};

} // namespace cmx

#endif
