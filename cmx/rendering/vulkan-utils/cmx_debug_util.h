#ifndef CMX_DEBUG_UTIL
#define CMX_DEBUG_UTIL

#include <vulkan/vulkan.hpp>

namespace cmx
{

class DebugUtil
{
  public:
    static void initialize(vk::Device device, vk::Instance instance)
    {
        _device = device;
        _dispatch = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);
    }

    static void setObjectName(uint64_t handle, vk::ObjectType objectType, const std::string &name)
    {
        if (!_device)
            return;

        vk::DebugUtilsObjectNameInfoEXT nameInfo{objectType, handle, name.c_str()};

        _device.setDebugUtilsObjectNameEXT(nameInfo, _dispatch);
    }

    template <typename T> static void nameObject(const T &object, vk::ObjectType objectType, const std::string &name)
    {
        setObjectName(reinterpret_cast<uint64_t>(static_cast<typename T::CType>(object)), objectType, name);
    }

  private:
    static inline vk::Device _device;
    static inline vk::DispatchLoaderDynamic _dispatch;
};

} // namespace cmx

#endif
