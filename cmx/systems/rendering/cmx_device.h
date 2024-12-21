#pragma once

#include "cmx_window.h"

// std lib headers
#include <vector>
#include <vulkan/vulkan_core.h>

namespace cmx
{

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
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

class CmxDevice
{
  public:
#ifdef NDEBUG
    const bool _enableValidationLayers = false;
#else
    const bool _enableValidationLayers = true;
#endif

    CmxDevice(CmxWindow &);
    ~CmxDevice();

    CmxDevice(const CmxDevice &) = delete;
    CmxDevice &operator=(const CmxDevice &) = delete;

    VkCommandPool getCommandPool()
    {
        return _commandPool;
    }
    VkDevice device()
    {
        return _device;
    }
    VkSurfaceKHR surface()
    {
        return _surface;
    }
    VkQueue graphicsQueue()
    {
        return _graphicsQueue;
    }
    VkQueue presentQueue()
    {
        return _presentQueue;
    }
    VkPhysicalDevice physicalDevice()
    {
        return _physicalDevice;
    }
    VkInstance instance()
    {
        return _instance;
    }

    SwapChainSupportDetails getSwapChainSupport()
    {
        return querySwapChainSupport(_physicalDevice);
    }
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags);
    QueueFamilyIndices findPhysicalQueueFamilies()
    {
        return findQueueFamilies(_physicalDevice);
    }
    VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling, VkFormatFeatureFlags);

    // Buffer Helper Functions
    void createBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkBuffer &, VkDeviceMemory &);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer);
    void copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize);
    void copyBufferToImage(VkBuffer, VkImage, uint32_t width, uint32_t height, uint32_t layerCount);

    void createImageWithInfo(const VkImageCreateInfo &, VkMemoryPropertyFlags, VkImage &, VkDeviceMemory &);

    VkPhysicalDeviceProperties _properties;

  private:
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createCommandPool();

    // helper functions
    bool isDeviceSuitable(VkPhysicalDevice);
    std::vector<const char *> getRequiredExtensions();
    bool checkValidationLayerSupport();
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice);
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &);
    void hasGflwRequiredInstanceExtensions();
    bool checkDeviceExtensionSupport(VkPhysicalDevice);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice);

    VkInstance _instance;
    VkDebugUtilsMessengerEXT _debugMessenger;
    VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
    CmxWindow &_window;
    VkCommandPool _commandPool;

    VkDevice _device;
    VkSurfaceKHR _surface;
    VkQueue _graphicsQueue;
    VkQueue _presentQueue;

    const std::vector<const char *> _validationLayers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char *> _deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};

} // namespace cmx
