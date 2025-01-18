#include "cmx_renderer.h"

#include "cmx_swap_chain.h"
#include "cmx_window.h"

// lib
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <glm/ext/scalar_constants.hpp>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <unistd.h>
#include <vulkan/vulkan_core.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <memory>
#include <stdexcept>

namespace cmx
{

CmxRenderer::CmxRenderer(CmxWindow &cmxWindow, CmxDevice &cmxDevice) : _cmxWindow{cmxWindow}, _cmxDevice{cmxDevice}
{
    recreateSwapChain();
    createCommandBuffers();
}

CmxRenderer::~CmxRenderer()
{
    if (!_freed)
    {
        spdlog::error("CmxRenderer: forgot to free before deletion");
    }
}

void CmxRenderer::free()
{
    _cmxSwapChain->free();
    delete _cmxSwapChain.release();

    freeCommandBuffers();

    _freed = true;
    spdlog::info("CmxRenderer: freed");
}

void CmxRenderer::recreateSwapChain()
{
    VkExtent2D extent = _cmxWindow.getExtent();
    while (extent.width == 0 || extent.height == 0)
    {
        extent = _cmxWindow.getExtent();
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(_cmxDevice.device());

    if (_cmxSwapChain == nullptr)
    {
        _cmxSwapChain = std::make_unique<CmxSwapChain>(_cmxDevice, extent);
    }
    else
    {
        std::shared_ptr<CmxSwapChain> oldSwapChain = std::move(_cmxSwapChain);
        _cmxSwapChain = std::make_unique<CmxSwapChain>(_cmxDevice, extent, oldSwapChain);
        if (!oldSwapChain->compareSwapFormats(*_cmxSwapChain.get()))
        {
            throw std::runtime_error("Swap chain image (or depth) format  has changed!");
        }
    }
}

void CmxRenderer::createCommandBuffers()
{
    _commandBuffers.resize(CmxSwapChain::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = _cmxDevice.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(_commandBuffers.size());

    if (vkAllocateCommandBuffers(_cmxDevice.device(), &allocInfo, _commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void CmxRenderer::freeCommandBuffers()
{
    vkFreeCommandBuffers(_cmxDevice.device(), _cmxDevice.getCommandPool(),
                         static_cast<uint32_t>(_commandBuffers.size()), _commandBuffers.data());
    _commandBuffers.clear();
}

VkCommandBuffer CmxRenderer::beginFrame()
{
    assert(!_isFrameStarted && "Can't call beginFrame while already in progress");

    VkResult result = _cmxSwapChain->acquireNextImage(&_currentImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapChain();
        return nullptr;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image");
    }

    _isFrameStarted = true;

    auto commandBuffer = getCurrentCommandBuffer();
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    return commandBuffer;
}

void CmxRenderer::endFrame()
{
    assert(_isFrameStarted && "Can't call endFrame while frame is not in progress");

    auto commandBuffer = getCurrentCommandBuffer();

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }

    auto result = _cmxSwapChain->submitCommandBuffers(&commandBuffer, &_currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || _cmxWindow.wasWindowResized())
    {
        _cmxWindow.resetWindowResizedFlag();
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }

    _isFrameStarted = false;
    _currentFrameIndex = (_currentFrameIndex + 1) % CmxSwapChain::MAX_FRAMES_IN_FLIGHT;
}

void CmxRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
    assert(_isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
    assert(commandBuffer == getCurrentCommandBuffer() &&
           "Can't begin render pass on command buffer from a different frame");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = _cmxSwapChain->getRenderPass();
    renderPassInfo.framebuffer = _cmxSwapChain->getFrameBuffer(_currentImageIndex);

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = _cmxSwapChain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(_cmxSwapChain->getSwapChainExtent().width);
    viewport.height = static_cast<float>(_cmxSwapChain->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{{0, 0}, _cmxSwapChain->getSwapChainExtent()};
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void CmxRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
    assert(_isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
    assert(commandBuffer == getCurrentCommandBuffer() &&
           "Can't end render pass on command buffer from a different frame");

    vkCmdEndRenderPass(commandBuffer);
}

} // namespace cmx
