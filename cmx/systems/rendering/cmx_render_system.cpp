#include "cmx_render_system.h"

// cmx
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

RenderSystem::RenderSystem(CmxWindow &cmxWindow) : cmxWindow{&cmxWindow}
{
    cmxDevice = std::unique_ptr<CmxDevice>(new CmxDevice(cmxWindow));
    cmxRenderer = std::unique_ptr<CmxRenderer>(new CmxRenderer(cmxWindow, *cmxDevice.get()));
    uboBuffers = std::vector<std::unique_ptr<CmxBuffer>>{CmxSwapChain::MAX_FRAMES_IN_FLIGHT};
    globalDescriptorSets = std::vector<VkDescriptorSet>{CmxSwapChain::MAX_FRAMES_IN_FLIGHT};
}

RenderSystem::~RenderSystem()
{
    vkDestroyPipelineLayout(cmxDevice->device(), pipelineLayout, nullptr);
}

void RenderSystem::initialize()
{
    globalPool = CmxDescriptorPool::Builder(*cmxDevice.get())
                     .setMaxSets(CmxSwapChain::MAX_FRAMES_IN_FLIGHT)
                     .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, CmxSwapChain::MAX_FRAMES_IN_FLIGHT)
                     .build();

    for (int i = 0; i < uboBuffers.size(); i++)
    {
        uboBuffers[i] =
            std::make_unique<CmxBuffer>(*cmxDevice.get(), sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        uboBuffers[i]->map();
    }

    auto globalSetLayout = CmxDescriptorSetLayout::Builder(*cmxDevice.get())
                               .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                               .build();
    for (int i = 0; i < globalDescriptorSets.size(); i++)
    {
        auto bufferInfo = uboBuffers[i]->descriptorInfo();
        CmxDescriptorWriter(*globalSetLayout, *globalPool).writeBuffer(0, &bufferInfo).build(globalDescriptorSets[i]);
    }

    createPipelineLayout(globalSetLayout->getDescriptorSetLayout());
    createPipeline(cmxRenderer->getSwapChainRenderPass());
}

void RenderSystem::drawScene(std::weak_ptr<CameraComponent> cameraWk,
                             const std::vector<std::shared_ptr<class Component>> &renderQueue)
{
    if (auto camera = cameraWk.lock())
    {
        float aspect = cmxRenderer->getAspectRatio();
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
        if (!noCameraFlag)
        {
            spdlog::warn("No active camera in scene");
            noCameraFlag = true;
        }
    }

    vkDeviceWaitIdle(cmxDevice->device());
}

FrameInfo *RenderSystem::beginRender(class CameraComponent *camera)
{
    FrameInfo *frameInfo;

    if ((commandBuffer = cmxRenderer->beginFrame()))
    {
        int frameIndex = cmxRenderer->getFrameIndex();
        frameInfo = new FrameInfo{frameIndex, commandBuffer, *camera, globalDescriptorSets[frameIndex]};
        // update
        GlobalUbo ubo{};
        ubo.projectionView = camera->getProjection() * camera->getView();
        uboBuffers[frameIndex]->writeToBuffer(&ubo);
        uboBuffers[frameIndex]->flush();

        // render
        cmxRenderer->beginSwapChainRenderPass(commandBuffer);
    }

    return frameInfo;
}

void RenderSystem::render(FrameInfo *frameInfo, const std::vector<std::shared_ptr<class Component>> &renderQueue)
{
    cmxPipeline->bind(frameInfo->commandBuffer);

    vkCmdBindDescriptorSets(frameInfo->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
                            &frameInfo->globalDescriptorSet, 0, nullptr);

    for (std::shared_ptr<Component> renderComponent : renderQueue)
    {
        renderComponent->render(*frameInfo, pipelineLayout);
    }
}

void RenderSystem::endRender()
{
    cmxRenderer->endSwapChainRenderPass(commandBuffer);
    cmxRenderer->endFrame();
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
    if (vkCreatePipelineLayout(cmxDevice->device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void RenderSystem::createPipeline(VkRenderPass renderPass)
{
    assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    CmxPipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = pipelineLayout;
    cmxPipeline = std::make_unique<CmxPipeline>(*cmxDevice.get(), "shaders/shader.vert.spv", "shaders/shader.frag.spv",
                                                pipelineConfig);
}

} // namespace cmx
