#pragma once

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
        return cmxSwapChain->getRenderPass();
    }

    bool isFrameInProgress() const
    {
        return isFrameStarted;
    }

    VkCommandBuffer getCurrentCommandBuffer() const
    {
        assert(isFrameStarted && "Cannot get command buffer when frame not in progress!");
        return commandBuffers[currentFrameIndex];
    }

    int getFrameIndex() const
    {
        assert(isFrameStarted && "Cannot get frame index when frame not in progress!");
        return currentFrameIndex;
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

    CmxWindow &cmxWindow;
    CmxDevice &cmxDevice;
    std::unique_ptr<CmxSwapChain> cmxSwapChain;
    std::vector<VkCommandBuffer> commandBuffers;

    uint32_t currentImageIndex;
    int currentFrameIndex{0};
    bool isFrameStarted{false};
};

} // namespace cmx
