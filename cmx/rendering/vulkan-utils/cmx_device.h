#ifndef CMX_DEVICE
#define CMX_DEVICE

// cmx
#include "cmx_window.h"

// std
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace cmx
{

struct SwapChainSupportDetails
{
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

struct QueueFamilyIndices
{
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    bool graphicsFamilyHasValue = false;
    bool presentFamilyHasValue = false;
    bool isComplete()
    {
        return graphicsFamilyHasValue && presentFamilyHasValue;
    }
};

class Device
{
  public:
#ifdef NDEBUG
    const bool _enableValidationLayers = false;
#else
    const bool _enableValidationLayers = true;
#endif

    Device(Window &);
    ~Device();

    Device(const Device &) = delete;
    Device &operator=(const Device &) = delete;

    vk::CommandPool getCommandPool()
    {
        return _commandPool;
    }
    vk::Device device()
    {
        return _device;
    }
    vk::SurfaceKHR surface()
    {
        return _surface;
    }
    vk::Queue graphicsQueue()
    {
        return _graphicsQueue;
    }
    vk::Queue presentQueue()
    {
        return _presentQueue;
    }
    vk::PhysicalDevice physicalDevice()
    {
        return _physicalDevice;
    }
    vk::Instance instance()
    {
        return _instance;
    }
    vk::SampleCountFlagBits getSampleCount();

    SwapChainSupportDetails getSwapChainSupport()
    {
        return querySwapChainSupport(_physicalDevice);
    }
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags) const;
    QueueFamilyIndices findPhysicalQueueFamilies() const
    {
        return findQueueFamilies(_physicalDevice);
    }
    vk::Format findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling, vk::FormatFeatureFlags);

    // Buffer Helper Functions
    void createBuffer(vk::DeviceSize, vk::BufferUsageFlags, vk::MemoryPropertyFlags, vk::Buffer &, vk::DeviceMemory &);
    vk::CommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(vk::CommandBuffer);
    void copyBuffer(vk::Buffer src, vk::Buffer dst, vk::DeviceSize);
    void copyBufferToImage(vk::Buffer, vk::Image, uint32_t width, uint32_t height, uint32_t depth = 1,
                           uint32_t layerCount = 1);

    void createImageWithInfo(const vk::ImageCreateInfo &, vk::MemoryPropertyFlags, vk::Image &,
                             vk::DeviceMemory &) const;
    void transitionImageLayout(vk::Image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels);

    vk::PhysicalDeviceProperties _properties;

  private:
    VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                          const VkAllocationCallbacks *pAllocator,
                                          VkDebugUtilsMessengerEXT *pDebugMessenger);
    void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                       const VkAllocationCallbacks *pAllocator);
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createCommandPool();

    // helper functions
    bool isDeviceSuitable(vk::PhysicalDevice);
    std::vector<const char *> getRequiredExtensions();
    bool checkValidationLayerSupport();
    QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice) const;
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &);
    void hasGflwRequiredInstanceExtensions();
    bool checkDeviceExtensionSupport(vk::PhysicalDevice);
    SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice);

    vk::Instance _instance;
    VkDebugUtilsMessengerEXT _debugMessenger;
    vk::PhysicalDevice _physicalDevice = nullptr;
    Window &_window;
    vk::CommandPool _commandPool;

    vk::Device _device;
    vk::SurfaceKHR _surface;
    vk::Queue _graphicsQueue;
    vk::Queue _presentQueue;

    const std::vector<const char *> _validationLayers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char *> _deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_MAINTENANCE_2_EXTENSION_NAME,
    };
};

} // namespace cmx

#endif
