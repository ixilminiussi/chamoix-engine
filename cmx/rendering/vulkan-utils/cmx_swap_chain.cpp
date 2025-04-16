#include "cmx_swap_chain.h"

// cmx
#include "cmx_debug_util.h"
#include "cmx_render_system.h"

// lib
#include <spdlog/spdlog.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

// std
#include <array>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_enums.hpp>

namespace cmx
{

SwapChain::SwapChain(Device &deviceRef, vk::Extent2D extent) : _device{deviceRef}, _windowExtent{extent}
{
    init();
}

SwapChain::SwapChain(Device &deviceRef, vk::Extent2D extent, std::shared_ptr<SwapChain> previous)
    : _device{deviceRef}, _windowExtent{extent}, _oldSwapChain{previous}
{
    init();

    // clean up old swap chain since it's no longer needed
    _oldSwapChain = nullptr;
}

void SwapChain::init()
{
    createSwapChain();
    createImageViews();
    createRenderPass();
    createFramebuffers();
    createSyncObjects();
}

SwapChain::~SwapChain()
{
    if (!_freed)
    {
        spdlog::error("SwapChain: forgot to free before deletion");
    }
}

void SwapChain::free()
{
    for (vk::Framebuffer framebuffer : _swapChainFramebuffers)
    {
        vkDestroyFramebuffer(_device.device(), framebuffer, nullptr);
    }
    _swapChainFramebuffers.clear();

    vkDestroyRenderPass(_device.device(), _renderPass, nullptr);

    for (int i = 0; i < _swapChainImageViews.size(); i++)
    {
        vkDestroyImageView(_device.device(), _swapChainImageViews[i], nullptr);
    }
    _swapChainImageViews.clear();

    if (_swapChain != nullptr)
    {
        vkDestroySwapchainKHR(_device.device(), _swapChain, nullptr);
        _swapChain = nullptr;
    }

    // cleanup synchronization objects
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(_device.device(), _renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(_device.device(), _imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(_device.device(), _inFlightFences[i], nullptr);
    }

    _freed = true;
}

vk::Result SwapChain::acquireNextImage(uint32_t *imageIndex)
{
    if (_device.device().waitForFences(1, &_inFlightFences[_currentFrame], VK_TRUE,
                                       std::numeric_limits<uint64_t>::max()) != vk::Result::eSuccess)
    {
        spdlog::warn("SwapChain: failed to wait for fences");
    }

    vk::Result result = _device.device().acquireNextImageKHR(
        _swapChain, std::numeric_limits<uint64_t>::max(),
        _imageAvailableSemaphores[_currentFrame], // must be a not signaled semaphore
        VK_NULL_HANDLE, imageIndex);

    return result;
}

vk::Result SwapChain::submitCommandBuffers(const vk::CommandBuffer *buffers, uint32_t *imageIndex)
{
    if (_imagesInFlight[*imageIndex] != VK_NULL_HANDLE)
    {
        if (_device.device().waitForFences(1, &_imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX) !=
            vk::Result::eSuccess)
        {
            spdlog::warn("SwapChain: failed to wait for fence");
        }
    }
    _imagesInFlight[*imageIndex] = _inFlightFences[_currentFrame];

    vk::SubmitInfo submitInfo = {};
    submitInfo.sType = vk::StructureType::eSubmitInfo;

    vk::Semaphore waitSemaphores[] = {_imageAvailableSemaphores[_currentFrame]};
    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = buffers;

    vk::Semaphore signalSemaphores[] = {_renderFinishedSemaphores[_currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (_device.device().resetFences(1, &_inFlightFences[_currentFrame]) != vk::Result::eSuccess)
    {
        spdlog::warn("SwapChain: failed to reset fence");
    }
    if (_device.graphicsQueue().submit(1, &submitInfo, _inFlightFences[_currentFrame]) != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to submit draw command buffer");
    }

    vk::PresentInfoKHR presentInfo = {};
    presentInfo.sType = vk::StructureType::ePresentInfoKHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    vk::SwapchainKHR swapChains[] = {_swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = imageIndex;

    vk::Result result = _device.presentQueue().presentKHR(&presentInfo);

    _currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    return result;
}

void SwapChain::createSwapChain()
{
    SwapChainSupportDetails swapChainSupport = _device.getSwapChainSupport();

    vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = vk::StructureType::eSwapchainCreateInfoKHR;
    createInfo.surface = _device.surface();

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

    QueueFamilyIndices indices = _device.findPhysicalQueueFamilies();
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
        createInfo.queueFamilyIndexCount = 0;     // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = _oldSwapChain == nullptr ? VK_NULL_HANDLE : _oldSwapChain->_swapChain;

    if (_device.device().createSwapchainKHR(&createInfo, nullptr, &_swapChain) != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to create swap chain!");
    }

    // we only specified a minimum number of images in the swap chain, so the implementation is
    // allowed to create a swap chain with more. That's why we'll first query the final number of
    // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
    // retrieve the handles.
    if (_device.device().getSwapchainImagesKHR(_swapChain, &imageCount, nullptr) != vk::Result::eSuccess)
    {
        spdlog::warn("swapchain: failed to get swapchainimageskhr");
    }
    _swapChainImages.resize(imageCount);
    if (_device.device().getSwapchainImagesKHR(_swapChain, &imageCount, _swapChainImages.data()) !=
        vk::Result::eSuccess)
    {
        spdlog::warn("swapchain: failed to get swapchainimageskhr");
    }

    _swapChainImageFormat = surfaceFormat.format;
    _swapChainExtent = extent;
}

void SwapChain::createImageViews()
{
    _swapChainImageViews.resize(_swapChainImages.size());
    for (size_t i = 0; i < _swapChainImages.size(); i++)
    {
        vk::ImageViewCreateInfo viewInfo{};
        viewInfo.sType = vk::StructureType::eImageViewCreateInfo;
        viewInfo.image = _swapChainImages[i];
        viewInfo.viewType = vk::ImageViewType::e2D;
        viewInfo.format = _swapChainImageFormat;
        viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (_device.device().createImageView(&viewInfo, nullptr, &_swapChainImageViews[i]) != vk::Result::eSuccess)
        {
            throw std::runtime_error("failed to create texture image view!");
        }
    }
}

void SwapChain::createRenderPass()
{
    vk::AttachmentDescription colorAttachment = {};
    colorAttachment.format = getSwapChainImageFormat();
    colorAttachment.samples = vk::SampleCountFlagBits::e1;
    colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
    colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

    vk::AttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::SubpassDescription subpass = {};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = nullptr;

    vk::SubpassDependency dependency = {};
    dependency.dstSubpass = 0;
    dependency.dstAccessMask =
        vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
    dependency.dstStageMask =
        vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
    dependency.srcSubpass = vk::SubpassExternal;
    dependency.srcAccessMask = vk::AccessFlagBits::eNone;
    dependency.srcStageMask =
        vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;

    std::array<vk::AttachmentDescription, 1> attachments = {colorAttachment};
    vk::RenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = vk::StructureType::eRenderPassCreateInfo;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (_device.device().createRenderPass(&renderPassInfo, nullptr, &_renderPass) != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to create render pass!");
    }

    DebugUtil::nameObject(_renderPass, vk::ObjectType::eRenderPass, "SwapChain RenderPass");
}

void SwapChain::createFramebuffers()
{
    _swapChainFramebuffers.resize(imageCount());
    for (size_t i = 0; i < imageCount(); i++)
    {
        std::array<vk::ImageView, 1> attachments = {_swapChainImageViews[i]};

        vk::Extent2D swapChainExtent = getSwapChainExtent();
        vk::FramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = vk::StructureType::eFramebufferCreateInfo;
        framebufferInfo.renderPass = _renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (_device.device().createFramebuffer(&framebufferInfo, nullptr, &_swapChainFramebuffers[i]) !=
            vk::Result::eSuccess)
        {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void SwapChain::createSyncObjects()
{
    _imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    _imagesInFlight.resize(imageCount(), VK_NULL_HANDLE);

    vk::SemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = vk::StructureType::eSemaphoreCreateInfo;

    vk::FenceCreateInfo fenceInfo = {};
    fenceInfo.sType = vk::StructureType::eFenceCreateInfo;
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (_device.device().createSemaphore(&semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]) !=
                vk::Result::eSuccess ||
            _device.device().createSemaphore(&semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]) !=
                vk::Result::eSuccess ||
            _device.device().createFence(&fenceInfo, nullptr, &_inFlightFences[i]) != vk::Result::eSuccess)
        {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

vk::SurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats)
{
    for (const vk::SurfaceFormatKHR &availableFormat : availableFormats)
    {
        if (availableFormat.format == vk::Format::eR8G8B8A8Srgb &&
            availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

vk::PresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes)
{
    for (const vk::PresentModeKHR &availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox)
        {
            spdlog::info("Present mode: Mailbox");
            return availablePresentMode;
        }
    }

    // for (const vk::PresentModeKHR &availablePresentMode : availablePresentModes) {
    //   if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
    //     spdlog::info("Present mode: Immediate");
    //     return availablePresentMode;
    //   }
    // }

    spdlog::info("present mode: V-sync");
    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D SwapChain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        vk::Extent2D actualExtent = _windowExtent;
        actualExtent.width = std::max(capabilities.minImageExtent.width,
                                      std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height,
                                       std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

vk::Format SwapChain::findDepthFormat()
{
    return _device.findSupportedFormat(
        {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint}, vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

} // namespace cmx
