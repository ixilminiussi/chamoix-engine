#ifndef CMX_SWAP_CHAIN
#define CMX_SWAP_CHAIN

#include "cmx_device.h"

// lib
#include <memory>
#include <vulkan/vulkan.h>

// std
#include <vector>

namespace cmx
{

class CmxSwapChain
{
  public:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    CmxSwapChain(CmxDevice &deviceRef, VkExtent2D windowExtent);
    CmxSwapChain(CmxDevice &deviceRef, VkExtent2D windowExtent, std::shared_ptr<CmxSwapChain> previous);
    ~CmxSwapChain();

    CmxSwapChain(const CmxSwapChain &) = delete;
    CmxSwapChain &operator=(const CmxSwapChain &) = delete;

    void free();

    VkFramebuffer getFrameBuffer(int index)
    {
        return _swapChainFramebuffers[index];
    }
    VkRenderPass getRenderPass()
    {
        return _renderPass;
    }
    VkImageView getImageView(int index)
    {
        return _swapChainImageViews[index];
    }
    size_t imageCount()
    {
        return _swapChainImages.size();
    }
    VkFormat getSwapChainImageFormat()
    {
        return _swapChainImageFormat;
    }
    VkExtent2D getSwapChainExtent()
    {
        return _swapChainExtent;
    }
    uint32_t width()
    {
        return _swapChainExtent.width;
    }
    uint32_t height()
    {
        return _swapChainExtent.height;
    }

    float extentAspectRatio()
    {
        return static_cast<float>(_swapChainExtent.width) / static_cast<float>(_swapChainExtent.height);
    }
    VkFormat findDepthFormat();

    VkResult acquireNextImage(uint32_t *imageIndex);
    VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

    bool compareSwapFormats(const CmxSwapChain &swapChain) const
    {
        return (_swapChainDepthFormat == swapChain._swapChainDepthFormat &&
                _swapChainImageFormat == swapChain._swapChainImageFormat);
    }

  private:
    void init();
    void createSwapChain();
    void createImageViews();
    void createDepthResources();
    void createRenderPass();
    void createFramebuffers();
    void createSyncObjects();

    // Helper functions
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    VkFormat _swapChainImageFormat;
    VkFormat _swapChainDepthFormat;
    VkExtent2D _swapChainExtent;

    std::vector<VkFramebuffer> _swapChainFramebuffers;
    VkRenderPass _renderPass;

    std::vector<VkImage> _depthImages;
    std::vector<VkDeviceMemory> _depthImageMemories;
    std::vector<VkImageView> _depthImageViews;
    std::vector<VkImage> _swapChainImages;
    std::vector<VkImageView> _swapChainImageViews;

    VkExtent2D _windowExtent;

    VkSwapchainKHR _swapChain;
    std::shared_ptr<CmxSwapChain> _oldSwapChain;

    std::vector<VkSemaphore> _imageAvailableSemaphores;
    std::vector<VkSemaphore> _renderFinishedSemaphores;
    std::vector<VkFence> _inFlightFences;
    std::vector<VkFence> _imagesInFlight;
    size_t _currentFrame = 0;

    CmxDevice &_cmxDevice;

    bool _freed{};
};

} // namespace cmx

#endif
