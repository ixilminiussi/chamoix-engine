#ifndef CMX_RENDERER
#define CMX_RENDERER

#include "cmx_device.h"
#include "cmx_swap_chain.h"
#include "cmx_window.h"

// lib
#include <vulkan/vulkan.hpp>

// std
#include <cassert>
#include <memory>
#include <vector>

namespace cmx
{

class Renderer
{
  public:
    Renderer(Window &, Device &);
    ~Renderer();

    void free();

    // getters and setters :: begin
    vk::RenderPass getSwapChainRenderPass() const
    {
        return _swapChain->getRenderPass();
    }

    float getAspectRatio() const
    {
        return _swapChain->extentAspectRatio();
    }

    bool isFrameInProgress() const
    {
        return _isFrameStarted;
    }

    vk::CommandBuffer getCurrentCommandBuffer() const
    {
        assert(_isFrameStarted && "Cannot get command buffer when frame not in progress!");
        return _commandBuffers[_currentFrameIndex];
    }

    int getFrameIndex() const
    {
        assert(_isFrameStarted && "Cannot get frame index when frame not in progress!");
        return _currentFrameIndex;
    }
    // getters and setters :: end

    Renderer(const Renderer &) = delete;
    Renderer &operator=(const Renderer &) = delete;

    vk::CommandBuffer beginFrame();
    void endFrame();
    void beginSwapChainRenderPass(vk::CommandBuffer);
    void endSwapChainRenderPass(vk::CommandBuffer);

  private:
    void createCommandBuffers();
    void freeCommandBuffers();
    void recreateSwapChain();

    Window &_window;
    Device &_device;
    std::unique_ptr<SwapChain> _swapChain;
    std::vector<vk::CommandBuffer> _commandBuffers;

    uint32_t _currentImageIndex;
    int _currentFrameIndex{0};
    bool _isFrameStarted{false};

    bool _freed{false};
};

} // namespace cmx

#endif
