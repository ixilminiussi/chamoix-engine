#include "cmx_render_system.h"

// cmx
#include "cmx_buffer.h"
#include "cmx_camera.h"
#include "cmx_descriptors.h"
#include "cmx_device.h"
#include "cmx_frame_info.h"
#include "cmx_game.h"
#include "cmx_pipeline.h"
#include "cmx_renderer.h"
#include "cmx_window.h"
#include "imgui.h"
#include <cstdlib>
#include <vulkan/vulkan_core.h>

namespace cmx
{

uint8_t RenderSystem::_activeSystem = NULL_RENDER_SYSTEM;

VkCommandBuffer RenderSystem::_commandBuffer = (VkCommandBuffer_T *)(0);
CmxWindow *RenderSystem::_cmxWindow = &Game::getWindow();
std::unique_ptr<CmxDevice> RenderSystem::_cmxDevice = std::make_unique<CmxDevice>(*_cmxWindow);
std::unique_ptr<CmxRenderer> RenderSystem::_cmxRenderer = std::make_unique<CmxRenderer>(*_cmxWindow, *_cmxDevice.get());
std::vector<std::unique_ptr<CmxBuffer>> RenderSystem::_uboBuffers =
    std::vector<std::unique_ptr<CmxBuffer>>{CmxSwapChain::MAX_FRAMES_IN_FLIGHT};
std::vector<VkDescriptorSet> RenderSystem::_globalDescriptorSets =
    std::vector<VkDescriptorSet>{CmxSwapChain::MAX_FRAMES_IN_FLIGHT};

RenderSystem::RenderSystem()
{
}

RenderSystem::~RenderSystem()
{
    delete _cmxPipeline.release();

    delete _globalPool.release();

    vkDestroyPipelineLayout(_cmxDevice->device(), _pipelineLayout, nullptr);
}

void RenderSystem::closeWindow()
{
    spdlog::info("global release");
    _cmxRenderer->free();
    delete _cmxRenderer.release();

    // vkFreeCommandBuffers(_cmxDevice->device(), _cmxDevice->getCommandPool(), 1u, &_commandBuffer);

    auto it = _uboBuffers.begin();
    while (it != _uboBuffers.end())
    {
        delete (*it).release();
        it++;
    }

    delete _cmxDevice.release();
    delete _cmxWindow;
}

void RenderSystem::editor(int i)
{
    ImGui::PushID(i);
    ImGui::Checkbox("##", &_visible);
    ImGui::PopID();
}

void RenderSystem::checkAspectRatio(class Camera *camera)
{
    float aspect = _cmxRenderer->getAspectRatio();
    camera->updateAspectRatio(aspect);
}

FrameInfo *RenderSystem::beginRender(Camera *camera, PointLight pointLights[MAX_POINT_LIGHTS], int numLights)
{
    FrameInfo *frameInfo;

    if ((_commandBuffer = _cmxRenderer->beginFrame()))
    {
        int frameIndex = _cmxRenderer->getFrameIndex();
        frameInfo = new FrameInfo{frameIndex, _commandBuffer, *camera, _globalDescriptorSets[frameIndex]};
        // update
        GlobalUbo ubo{};
        ubo.projection = camera->getProjection();
        ubo.view = camera->getView();

        for (int i = 0; i < numLights; i++)
        {
            ubo.pointLights[i].position = pointLights[i].position;
            ubo.pointLights[i].color = pointLights[i].color;
        }

        ubo.numLights = numLights;

        _uboBuffers[frameIndex]->writeToBuffer(&ubo);
        _uboBuffers[frameIndex]->flush();

        // render
        _cmxRenderer->beginSwapChainRenderPass(_commandBuffer);
    }

    return frameInfo;
}

void RenderSystem::endRender()
{
    _activeSystem = NULL_RENDER_SYSTEM;

    _cmxRenderer->endSwapChainRenderPass(_commandBuffer);
    _cmxRenderer->endFrame();
    vkDeviceWaitIdle(_cmxDevice->device());
}

CmxDevice *RenderSystem::getDevice()
{
    return _cmxDevice.get();
}

} // namespace cmx
