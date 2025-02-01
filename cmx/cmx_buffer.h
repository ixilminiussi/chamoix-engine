#ifndef CMX_BUFFER
#define CMX_BUFFER

#include "cmx_device.h"

namespace cmx
{

class Buffer
{
  public:
    Buffer(Device &device, vk::DeviceSize instanceSize, uint32_t instanceCount, vk::BufferUsageFlags usageFlags,
           vk::MemoryPropertyFlags memoryPropertyFlags, vk::DeviceSize minOffsetAlignment = 1);
    ~Buffer();
    void free();

    Buffer(const Buffer &) = delete;
    Buffer &operator=(const Buffer &) = delete;

    vk::Result map(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
    void unmap();

    void writeToBuffer(void *data, vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
    vk::Result flush(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
    vk::DescriptorBufferInfo descriptorInfo(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
    vk::Result invalidate(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);

    void writeToIndex(void *data, int index);
    vk::Result flushIndex(int index);
    vk::DescriptorBufferInfo descriptorInfoForIndex(int index);
    vk::Result invalidateIndex(int index);

    const vk::Buffer &getBuffer() const
    {
        return _buffer;
    }
    void *getMappedMemory() const
    {
        return _mapped;
    }
    uint32_t getInstanceCount() const
    {
        return _instanceCount;
    }
    vk::DeviceSize getInstanceSize() const
    {
        return _instanceSize;
    }
    vk::DeviceSize getAlignmentSize() const
    {
        return _instanceSize;
    }
    vk::BufferUsageFlags getUsageFlags() const
    {
        return _usageFlags;
    }
    vk::MemoryPropertyFlags getMemoryPropertyFlags() const
    {
        return _memoryPropertyFlags;
    }
    vk::DeviceSize getBufferSize() const
    {
        return _bufferSize;
    }

  private:
    static vk::DeviceSize getAlignment(vk::DeviceSize instanceSize, vk::DeviceSize minOffsetAlignment);

    Device &_device;
    void *_mapped = nullptr;
    vk::Buffer _buffer = VK_NULL_HANDLE;
    vk::DeviceMemory _memory = VK_NULL_HANDLE;

    vk::DeviceSize _bufferSize;
    uint32_t _instanceCount;
    vk::DeviceSize _instanceSize;
    vk::DeviceSize _alignmentSize;
    vk::BufferUsageFlags _usageFlags;
    vk::MemoryPropertyFlags _memoryPropertyFlags;

    bool _freed{false};
};

} // namespace cmx

#endif
