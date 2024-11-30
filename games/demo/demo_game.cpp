#include "demo_game.h"

#include "cmx_actor.h"
#include "cmx_camera_component.h"
#include "cmx_default_render_system.h"
#include "cmx_input_manager.h"
#include "cmx_model.h"
#include "cmx_render_component.h"
#include "cmx_world.h"
#include "cube_actor.h"

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

                std::vector<std::weak_ptr<cmx::Component>> &components = getWorld()->getAllComponents();
                defaultRenderSystem.render(commandBuffer, components, *camera.get());
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

std::unique_ptr<cmx::CmxModel> createCubeModel(cmx::CmxDevice &device, glm::vec3 offset)
{
    CmxModel::Builder modelBuilder{};

    modelBuilder.vertices = {
        // left face (white)
        {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},

        // right face (yellow)
        {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},

        // top face (orange, remember y axis points down)
        {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},

        // bottom face (red)
        {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},

        // nose face (blue)
        {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},

        // tail face (green)
        {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
    };
    for (auto &v : modelBuilder.vertices)
    {
        v.position += offset;
    }

    modelBuilder.indices = {0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
                            12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21};

    return std::make_unique<CmxModel>(device, modelBuilder);
}

void Demo::load()
{
    createInputManager(cmxWindow, {});
    setWorld(&mainWorld);
    getInputManager()->bindButton("exit", [](float val) { std::exit(EXIT_SUCCESS); });

    std::shared_ptr<CubeActor> cubeActor = cmx::Actor::spawn<CubeActor>(getWorld(), "Cube Actor");

    std::shared_ptr<CmxModel> cubeModel = createCubeModel(cmxDevice, glm::vec3{0.f});

    auto cubeRendererWk = cubeActor->getComponentByType<cmx::RenderComponent>();
    if (auto cubeRendererComponent = cubeRendererWk.lock())
    {
        cubeRendererComponent->setModel(cubeModel);
    }
}
