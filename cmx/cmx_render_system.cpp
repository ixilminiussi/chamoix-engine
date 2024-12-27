#include "cmx_render_system.h"

// cmx
#include "cmx_buffer.h"
#include "cmx_camera.h"
#include "cmx_component.h"
#include "cmx_descriptors.h"
#include "cmx_device.h"
#include "cmx_frame_info.h"
#include "cmx_pipeline.h"
#include "cmx_renderer.h"
#include "cmx_window.h"

// lib
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <glm/ext/scalar_constants.hpp>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <memory>
#include <stdexcept>

namespace cmx
{

struct GlobalUbo
{
    glm::mat4 projectionView{1.f};
    glm::vec3 lightDirection = glm::normalize(glm::vec3{1.f, -3.f, -1.f});
};

RenderSystem::RenderSystem(CmxWindow &cmxWindow) : _cmxWindow{&cmxWindow}
{
    _cmxDevice = std::unique_ptr<CmxDevice>(new CmxDevice(cmxWindow));
    _cmxRenderer = std::unique_ptr<CmxRenderer>(new CmxRenderer(cmxWindow, *_cmxDevice.get()));
    _uboBuffers = std::vector<std::unique_ptr<CmxBuffer>>{CmxSwapChain::MAX_FRAMES_IN_FLIGHT};
    _globalDescriptorSets = std::vector<VkDescriptorSet>{CmxSwapChain::MAX_FRAMES_IN_FLIGHT};
}

RenderSystem::~RenderSystem()
{
    vkDestroyPipelineLayout(_cmxDevice->device(), _pipelineLayout, nullptr);
}

void RenderSystem::initialize()
{
    _globalPool = CmxDescriptorPool::Builder(*_cmxDevice.get())
                      .setMaxSets(CmxSwapChain::MAX_FRAMES_IN_FLIGHT)
                      .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, CmxSwapChain::MAX_FRAMES_IN_FLIGHT)
                      .build();

    for (int i = 0; i < _uboBuffers.size(); i++)
    {
        _uboBuffers[i] =
            std::make_unique<CmxBuffer>(*_cmxDevice.get(), sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        _uboBuffers[i]->map();
    }

    auto globalSetLayout = CmxDescriptorSetLayout::Builder(*_cmxDevice.get())
                               .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                               .build();
    for (int i = 0; i < _globalDescriptorSets.size(); i++)
    {
        auto bufferInfo = _uboBuffers[i]->descriptorInfo();
        CmxDescriptorWriter(*globalSetLayout, *_globalPool).writeBuffer(0, &bufferInfo).build(_globalDescriptorSets[i]);
    }

    createPipelineLayout(globalSetLayout->getDescriptorSetLayout());
    createPipeline(_cmxRenderer->getSwapChainRenderPass());
}

void RenderSystem::drawScene(std::weak_ptr<Camera> cameraWk,
                             const std::vector<std::shared_ptr<class Component>> &renderQueue)
{
    if (auto camera = cameraWk.lock())
    {
        float aspect = _cmxRenderer->getAspectRatio();
        camera->updateAspectRatio(aspect);

        FrameInfo *frameInfo = beginRender(camera.get());

        if (frameInfo)
        {
            render(frameInfo, renderQueue);
            endRender();
        }

        delete frameInfo;
    }
    else
    {
        if (!_noCameraFlag)
        {
            spdlog::warn("No active camera in scene");
            _noCameraFlag = true;
        }
    }

    vkDeviceWaitIdle(_cmxDevice->device());
}

FrameInfo *RenderSystem::beginRender(Camera *camera)
{
    FrameInfo *frameInfo;

    if ((_commandBuffer = _cmxRenderer->beginFrame()))
    {
        int frameIndex = _cmxRenderer->getFrameIndex();
        frameInfo = new FrameInfo{frameIndex, _commandBuffer, *camera, _globalDescriptorSets[frameIndex]};
        // update
        GlobalUbo ubo{};
        ubo.projectionView = camera->getProjection() * camera->getView();
        _uboBuffers[frameIndex]->writeToBuffer(&ubo);
        _uboBuffers[frameIndex]->flush();

        // render
        _cmxRenderer->beginSwapChainRenderPass(_commandBuffer);
    }

    return frameInfo;
}

void RenderSystem::render(FrameInfo *frameInfo, const std::vector<std::shared_ptr<Component>> &renderQueue)
{
    _cmxPipeline->bind(frameInfo->commandBuffer);

    vkCmdBindDescriptorSets(frameInfo->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1,
                            &frameInfo->globalDescriptorSet, 0, nullptr);

    for (std::shared_ptr<Component> renderComponent : renderQueue)
    {
        renderComponent->render(*frameInfo, _pipelineLayout);
    }
}

void RenderSystem::endRender()
{
    _cmxRenderer->endSwapChainRenderPass(_commandBuffer);
    _cmxRenderer->endFrame();
}

void RenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
{
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    if (vkCreatePipelineLayout(_cmxDevice->device(), &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void RenderSystem::createPipeline(VkRenderPass renderPass)
{
    assert(_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    CmxPipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = _pipelineLayout;
    _cmxPipeline = std::make_unique<CmxPipeline>(*_cmxDevice.get(), "shaders/shader.vert.spv",
                                                 "shaders/shader.frag.spv", pipelineConfig);
}

} // namespace cmx
