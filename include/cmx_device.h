#pragma once

#include "cmx_window.h"

// std lib headers
#include <vector>

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
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    CmxDevice(CmxWindow &);
    ~CmxDevice();

    CmxDevice(const CmxDevice &) = delete;
    CmxDevice &operator=(const CmxDevice &) = delete;

    VkCommandPool getCommandPool()
    {
        return commandPool;
    }
    VkDevice device()
    {
        return device_;
    }
    VkSurfaceKHR surface()
    {
        return surface_;
    }
    VkQueue graphicsQueue()
    {
        return graphicsQueue_;
    }
    VkQueue presentQueue()
    {
        return presentQueue_;
    }

    SwapChainSupportDetails getSwapChainSupport()
    {
        return querySwapChainSupport(physicalDevice);
    }
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags);
    QueueFamilyIndices findPhysicalQueueFamilies()
    {
        return findQueueFamilies(physicalDevice);
    }
    VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling, VkFormatFeatureFlags);

    // Buffer Helper Functions
    void createBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkBuffer &, VkDeviceMemory &);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer);
    void copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize);
    void copyBufferToImage(VkBuffer, VkImage, uint32_t width, uint32_t height, uint32_t layerCount);

    void createImageWithInfo(const VkImageCreateInfo &, VkMemoryPropertyFlags, VkImage &, VkDeviceMemory &);

    VkPhysicalDeviceProperties properties;

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

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    CmxWindow &window;
    VkCommandPool commandPool;

    VkDevice device_;
    VkSurfaceKHR surface_;
    VkQueue graphicsQueue_;
    VkQueue presentQueue_;

    const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};

} // namespace cmx
