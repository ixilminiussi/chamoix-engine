#ifndef CMX_RENDERER
#define CMX_RENDERER

#include "cmx_device.h"
#include "cmx_swap_chain.h"
#include "cmx_window.h"

// lib
#include <vulkan/vulkan_core.h>

// std
#include <cassert>
#include <memory>
#include <vector>

namespace cmx
{

class CmxRenderer
{
  public:
    CmxRenderer(CmxWindow &, CmxDevice &);
    ~CmxRenderer();

    // getters and setters :: begin
    VkRenderPass getSwapChainRenderPass() const
    {
        return _cmxSwapChain->getRenderPass();
    }

    float getAspectRatio() const
    {
        return _cmxSwapChain->extentAspectRatio();
    }

    bool isFrameInProgress() const
    {
        return _isFrameStarted;
    }

    VkCommandBuffer getCurrentCommandBuffer() const
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

    CmxRenderer(const CmxRenderer &) = delete;
    CmxRenderer &operator=(const CmxRenderer &) = delete;

    VkCommandBuffer beginFrame();
    void endFrame();
    void beginSwapChainRenderPass(VkCommandBuffer);
    void endSwapChainRenderPass(VkCommandBuffer);

  private:
    void createCommandBuffers();
    void freeCommandBuffers();
    void recreateSwapChain();

    CmxWindow &_cmxWindow;
    CmxDevice &_cmxDevice;
    std::unique_ptr<CmxSwapChain> _cmxSwapChain;
    std::vector<VkCommandBuffer> _commandBuffers;

    uint32_t _currentImageIndex;
    int _currentFrameIndex{0};
    bool _isFrameStarted{false};
};

} // namespace cmx

#endif
