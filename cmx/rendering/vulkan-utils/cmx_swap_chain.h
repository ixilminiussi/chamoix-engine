#ifndef CMX_SWAP_CHAIN
#define CMX_SWAP_CHAIN

#include "cmx_device.h"

// lib
#include <memory>
#include <vulkan/vulkan.hpp>

// std
#include <vector>

namespace cmx
{

class SwapChain
{
  public:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    SwapChain(Device &deviceRef, vk::Extent2D windowExtent);
    SwapChain(Device &deviceRef, vk::Extent2D windowExtent, std::shared_ptr<SwapChain> previous);
    ~SwapChain();

    SwapChain(const SwapChain &) = delete;
    SwapChain &operator=(const SwapChain &) = delete;

    void free();

    vk::Framebuffer getFrameBuffer(int index)
    {
        return _swapChainFramebuffers[index];
    }
    vk::RenderPass getRenderPass()
    {
        return _renderPass;
    }
    vk::ImageView getImageView(int index)
    {
        return _swapChainImageViews[index];
    }
    size_t imageCount()
    {
        return _swapChainImages.size();
    }
    vk::Format getSwapChainImageFormat()
    {
        return _swapChainImageFormat;
    }
    vk::Extent2D getSwapChainExtent()
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
    vk::Format findDepthFormat();

    vk::Result acquireNextImage(uint32_t *imageIndex);
    vk::Result submitCommandBuffers(const vk::CommandBuffer *buffers, uint32_t *imageIndex);

    bool compareSwapFormats(const SwapChain &swapChain) const
    {
        return (_swapChainDepthFormat == swapChain._swapChainDepthFormat &&
                _swapChainImageFormat == swapChain._swapChainImageFormat);
    }

    static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats);

  private:
    void init();
    void createSwapChain();
    void createImageViews();
    void createDepthResources();
    void createRenderPass();
    void createFramebuffers();
    void createSyncObjects();

    // Helper functions
    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes);
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities);

    vk::Format _swapChainImageFormat;
    vk::Format _swapChainDepthFormat;
    vk::Extent2D _swapChainExtent;

    std::vector<vk::Framebuffer> _swapChainFramebuffers;
    vk::RenderPass _renderPass;

    std::vector<vk::Image> _depthImages;
    std::vector<vk::DeviceMemory> _depthImageMemories;
    std::vector<vk::ImageView> _depthImageViews;
    std::vector<vk::Image> _swapChainImages;
    std::vector<vk::ImageView> _swapChainImageViews;

    vk::Extent2D _windowExtent;

    vk::SwapchainKHR _swapChain;
    std::shared_ptr<SwapChain> _oldSwapChain;

    std::vector<vk::Semaphore> _imageAvailableSemaphores;
    std::vector<vk::Semaphore> _renderFinishedSemaphores;
    std::vector<vk::Fence> _inFlightFences;
    std::vector<vk::Fence> _imagesInFlight;
    size_t _currentFrame = 0;

    Device &_device;

    bool _freed{};
};

} // namespace cmx

#endif
