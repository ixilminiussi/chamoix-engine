#include "game.h"
#include "cmx_actor.h"
#include "cmx_model.h"
#include "cmx_pipeline.h"
#include "cmx_render_component.h"
#include "cmx_swap_chain.h"
#include "cmx_world.h"

// lib
#include <GLFW/glfw3.h>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <array>
#include <memory>
#include <stdexcept>

namespace cmx
{

struct SimplePushConstantData
{
    glm::mat2 transform{1.f};
    glm::vec2 offset;
    alignas(16) glm::vec3 color;
};

Game::Game()
{
    load();
    createPipelineLayout();
    recreateSwapChain();
    createCommandBuffers();
}

Game::~Game()
{
    vkDestroyPipelineLayout(cmxDevice.device(), pipelineLayout, nullptr);
}

void Game::run()
{
    while (!cmxWindow.shouldClose())
    {
        glfwPollEvents();
        drawFrame();
    }

    vkDeviceWaitIdle(cmxDevice.device());
}

void Game::load()
{
    std::vector<CmxModel::Vertex> vertices = {
        {{0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}}, {{0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}}, {{-0.5, 0.5}, {0.0f, 0.0f, 1.0f}}};

    auto cmxModel = std::make_shared<CmxModel>(cmxDevice, vertices);

    setWorld(&mainWorld);

    auto actor = Actor::spawn(&mainWorld, "triangle1");
    actor->setPosition(glm::vec3{0.0f});
    actor->setScale(glm::vec3{1.0f});

    auto renderComponent = std::make_shared<RenderComponent>(cmxModel);
    actor->attachComponent(renderComponent);
}

void Game::createPipelineLayout()
{
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    if (vkCreatePipelineLayout(cmxDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void Game::createPipeline()
{
    assert(cmxSwapChain != nullptr && "Cannot create pipeline before swap chain");
    assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    CmxPipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = cmxSwapChain->getRenderPass();
    pipelineConfig.pipelineLayout = pipelineLayout;
    cmxPipeline =
        std::make_unique<CmxPipeline>(cmxDevice, "shaders/shader.vert.spv", "shaders/shader.frag.spv", pipelineConfig);
}

void Game::recreateSwapChain()
{
    VkExtent2D extent = cmxWindow.getExtent();
    while (extent.width == 0 || extent.height == 0)
    {
        extent = cmxWindow.getExtent();
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(cmxDevice.device());

    if (cmxSwapChain == nullptr)
    {
        cmxSwapChain = std::make_unique<CmxSwapChain>(cmxDevice, extent);
    }
    else
    {
        cmxSwapChain = std::make_unique<CmxSwapChain>(cmxDevice, extent, std::move(cmxSwapChain));
        if (cmxSwapChain->imageCount() != commandBuffers.size())
        {
            freeCommandBuffers();
            createCommandBuffers();
        }
    }
    createPipeline();
}

void Game::createCommandBuffers()
{
    commandBuffers.resize(cmxSwapChain->imageCount());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = cmxDevice.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    if (vkAllocateCommandBuffers(cmxDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    for (int i = 0; i < commandBuffers.size(); i++)
    {
        recordCommandBuffer(i);
    }
}

void Game::freeCommandBuffers()
{
    vkFreeCommandBuffers(cmxDevice.device(), cmxDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()),
                         commandBuffers.data());
    commandBuffers.clear();
}

void Game::recordCommandBuffer(int imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = cmxSwapChain->getRenderPass();
    renderPassInfo.framebuffer = cmxSwapChain->getFrameBuffer(imageIndex);

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = cmxSwapChain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(cmxSwapChain->getSwapChainExtent().width);
    viewport.height = static_cast<float>(cmxSwapChain->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{{0, 0}, cmxSwapChain->getSwapChainExtent()};
    vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
    vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

    render(commandBuffers[imageIndex]);

    vkCmdEndRenderPass(commandBuffers[imageIndex]);
    if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void Game::render(VkCommandBuffer commandBuffer)
{
    if (!getWorld())
    {
        spdlog::throw_spdlog_ex("forgot to set active world");
    }
    cmxPipeline->bind(commandBuffer);

    std::vector<std::weak_ptr<Component>> &components = getWorld()->getAllComponents();

    auto j = components.begin();

    while (j < components.end())
    {
        if (j->expired())
        {
            j = components.erase(j);
            continue;
        }

        std::shared_ptr<Component> component = j->lock();
        if (component)
            component->render(commandBuffer, pipelineLayout);

        j++;
    }
}

void Game::drawFrame()
{
    uint32_t imageIndex;
    VkResult result = cmxSwapChain->acquireNextImage(&imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapChain();
        return;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image");
    }

    recordCommandBuffer(imageIndex);
    result = cmxSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || cmxWindow.wasWindowResized())
    {
        cmxWindow.resetWindowResizedFlag();
        recreateSwapChain();
        return;
    }
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }
}

} // namespace cmx
