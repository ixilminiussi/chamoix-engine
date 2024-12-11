#include "demo_game.h"

#include "cmx_actor.h"
#include "cmx_assets_manager.h"
#include "cmx_buffer.h"
#include "cmx_camera_component.h"
#include "cmx_default_render_system.h"
#include "cmx_descriptors.h"
#include "cmx_frame_info.h"
#include "cmx_input_action.h"
#include "cmx_mesh_component.h"
#include "cmx_model.h"
#include "cmx_scene.h"
#include "cmx_swap_chain.h"
#include "rotating_actor.h"
#include "tinyxml2.h"

// lib
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <glm/ext/scalar_constants.hpp>
#include <glm/geometric.hpp>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <memory>

struct GlobalUbo
{
    glm::mat4 projectionView{1.f};
    glm::vec3 lightDirection = glm::normalize(glm::vec3{1.f, -3.f, -1.f});
};

Demo::~Demo()
{
    vkDestroyPipelineLayout(cmxDevice.device(), pipelineLayout, nullptr);
}

void Demo::run()
{
    std::vector<std::unique_ptr<cmx::CmxBuffer>> uboBuffers(cmx::CmxSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < uboBuffers.size(); i++)
    {
        uboBuffers[i] = std::make_unique<cmx::CmxBuffer>(
            cmxDevice, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        uboBuffers[i]->map();
    }

    auto globalSetLayout = cmx::CmxDescriptorSetLayout::Builder(cmxDevice)
                               .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                               .build();
    std::vector<VkDescriptorSet> globalDescriptorSets(cmx::CmxSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < globalDescriptorSets.size(); i++)
    {
        auto bufferInfo = uboBuffers[i]->descriptorInfo();
        cmx::CmxDescriptorWriter(*globalSetLayout, *globalPool)
            .writeBuffer(0, &bufferInfo)
            .build(globalDescriptorSets[i]);
    }

    cmx::DefaultRenderSystem defaultRenderSystem{cmxDevice, cmxRenderer.getSwapChainRenderPass(),
                                                 globalSetLayout->getDescriptorSetLayout()};

    while (!cmxWindow.shouldClose())
    {
        float dt = glfwGetTime();
        glfwSetTime(0.);

        getInputManager()->pollEvents(dt);

        getScene()->updateActors(dt);
        getScene()->updateComponents(dt);

        if (auto camera = getScene()->getCamera().lock())
        {
            noCameraFlag = false;

            float aspect = cmxRenderer.getAspectRatio();
            camera->updateAspectRatio(aspect);

            if (auto commandBuffer = cmxRenderer.beginFrame())
            {
                int frameIndex = cmxRenderer.getFrameIndex();
                cmx::FrameInfo frameInfo{frameIndex, commandBuffer, *camera, globalDescriptorSets[frameIndex]};
                // update
                GlobalUbo ubo{};
                ubo.projectionView = camera->getProjection() * camera->getView();
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                // render
                cmxRenderer.beginSwapChainRenderPass(commandBuffer);
                std::vector<std::weak_ptr<cmx::Component>> &renderQueue = getScene()->getRenderQueue();
                defaultRenderSystem.render(frameInfo, renderQueue);

                // ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
                cmxRenderer.endSwapChainRenderPass(commandBuffer);
                cmxRenderer.endFrame();
            }
        }
        else
        {
            if (!noCameraFlag)
            {
                spdlog::warn("No active camera in scene '{0}'", getScene()->name);
                noCameraFlag = true;
            }
        }
    }

    vkDeviceWaitIdle(cmxDevice.device());
}

void Demo::closeWindow(float dt, int val)
{
    std::exit(EXIT_SUCCESS);
}

void Demo::load()
{
    globalPool = cmx::CmxDescriptorPool::Builder(cmxDevice)
                     .setMaxSets(cmx::CmxSwapChain::MAX_FRAMES_IN_FLIGHT)
                     .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, cmx::CmxSwapChain::MAX_FRAMES_IN_FLIGHT)
                     .build();

    inputManager->load();

    scenes.push_back(&mainScene);
    setScene(0);

    getInputManager()->bindButton("exit", &Demo::closeWindow, this);

    std::shared_ptr<RotatingActor> rotatingActor = cmx::Actor::spawn<RotatingActor>(getScene(), "RotatingActor");

    std::shared_ptr<RotatingActor> rotatingActor2 = cmx::Actor::spawn<RotatingActor>(getScene(), "RotatingActor2");

    auto rotatingRendererWk = rotatingActor->getComponentByType<cmx::MeshComponent>();
    if (auto rotatingRendererComponent = rotatingRendererWk.lock())
    {
        rotatingRendererComponent->setModel("bunny");
    }

    auto rotatingRendererWk2 = rotatingActor2->getComponentByType<cmx::MeshComponent>();
    if (auto rotatingRendererComponent = rotatingRendererWk2.lock())
    {
        rotatingRendererComponent->setModel("bunny");
    }
}
