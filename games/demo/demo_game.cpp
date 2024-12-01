#include "demo_game.h"

#include "cmx_actor.h"
#include "cmx_camera_component.h"
#include "cmx_default_render_system.h"
#include "cmx_input_action.h"
#include "cmx_input_manager.h"
#include "cmx_model.h"
#include "cmx_render_component.h"
#include "cmx_world.h"
#include "rotating_actor.h"

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

using cmx::CmxModel;

Demo::~Demo()
{
    vkDestroyPipelineLayout(cmxDevice.device(), pipelineLayout, nullptr);
}

void Demo::run()
{
    cmx::DefaultRenderSystem defaultRenderSystem{cmxDevice, cmxRenderer.getSwapChainRenderPass()};

    while (!cmxWindow.shouldClose())
    {
        float dt = glfwGetTime();
        glfwSetTime(0.);

        getInputManager()->pollEvents(dt);

        getWorld()->updateActors(dt);
        getWorld()->updateComponents(dt);

        if (auto camera = getWorld()->getCamera().lock())
        {
            float aspect = cmxRenderer.getAspectRatio();
            camera->updateAspectRatio(aspect);

            if (auto commandBuffer = cmxRenderer.beginFrame())
            {
                cmxRenderer.beginSwapChainRenderPass(commandBuffer);

                std::vector<std::weak_ptr<cmx::Component>> &renderQueue = getWorld()->getRenderQueue();
                defaultRenderSystem.render(commandBuffer, renderQueue, *camera.get());
                cmxRenderer.endSwapChainRenderPass(commandBuffer);
                cmxRenderer.endFrame();
            }
        }
        else
        {
            spdlog::warn("No active camera in world '{0}'", getWorld()->name);
        }
    }

    vkDeviceWaitIdle(cmxDevice.device());
}

void Demo::load()
{
    createInputManager(
        cmxWindow, {{"exit", new cmx::ButtonAction{cmx::ButtonAction::Type::RELEASED, {cmx::CMX_KEY_ESCAPE}}},
                    {"slowdown on", new cmx::ButtonAction{cmx::ButtonAction::Type::PRESSED, {cmx::CMX_KEY_SPACE}}},
                    {"slowdown off", new cmx::ButtonAction{cmx::ButtonAction::Type::RELEASED, {cmx::CMX_KEY_SPACE}}}});
    setWorld(&mainWorld);
    getInputManager()->bindButton("exit", [](float val) { std::exit(EXIT_SUCCESS); });

    std::shared_ptr<RotatingActor> rotatingActor = cmx::Actor::spawn<RotatingActor>(getWorld(), "Rotating Actor");
    rotatingActor->transform.scale = glm::vec3{10.f};

    std::shared_ptr<CmxModel> rotatingModel = CmxModel::createModelFromFile(cmxDevice, "assets/models/bunny.obj");

    auto rotatingRendererWk = rotatingActor->getComponentByType<cmx::RenderComponent>();
    if (auto rotatingRendererComponent = rotatingRendererWk.lock())
    {
        rotatingRendererComponent->setModel(rotatingModel);
    }
}
