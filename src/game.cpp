#include "game.h"

#include "cmx_actor.h"
#include "cmx_camera_component.h"
#include "cmx_default_render_system.h"
#include "cmx_model.h"
#include "cmx_render_component.h"
#include "cmx_world.h"
#include "cube_actor.h"
#include "viewport_actor.h"

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

namespace cmx
{

Game::Game()
{
    load();
}

Game::~Game()
{
    vkDestroyPipelineLayout(cmxDevice.device(), pipelineLayout, nullptr);
}

void Game::run()
{
    CmxDefaultRenderSystem defaultRenderSystem{cmxDevice, cmxRenderer.getSwapChainRenderPass()};

    while (!cmxWindow.shouldClose())
    {
        float dt = glfwGetTime();
        glfwSetTime(0.);
        glfwPollEvents();

        getWorld()->updateActors(dt);
        getWorld()->updateComponents(dt);

        if (auto camera = getWorld()->getCamera().lock())
        {
            float aspect = cmxRenderer.getAspectRatio();
            camera->updateAspectRatio(aspect);

            if (auto commandBuffer = cmxRenderer.beginFrame())
            {
                cmxRenderer.beginSwapChainRenderPass(commandBuffer);

                std::vector<std::weak_ptr<Component>> &components = getWorld()->getAllComponents();
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

std::unique_ptr<CmxModel> createCubeModel(CmxDevice &device, glm::vec3 offset)
{
    std::vector<CmxModel::Vertex> vertices{

        // left face (white)
        {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

        // right face (yellow)
        {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .8f, .1f}},

        // top face (orange, remember y axis points down)
        {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

        // bottom face (red)
        {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .1f, .1f}},

        // nose face (blue)
        {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

        // tail face (green)
        {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
    };
    for (auto &v : vertices)
    {
        v.position += offset;
    }
    CmxModel *model = new CmxModel(device, vertices);
    return std::unique_ptr<CmxModel>(model);
}

void Game::load()
{
    setWorld(&mainWorld);

    std::shared_ptr<ViewportActor> viewportActor = Actor::spawn<ViewportActor>(getWorld(), "Viewport Actor");

    getWorld()->setCamera(viewportActor->getCamera().lock());

    std::shared_ptr<CubeActor> cubeActor = Actor::spawn<CubeActor>(getWorld(), "Cube Actor");

    std::shared_ptr<CmxModel> cubeModel = createCubeModel(cmxDevice, glm::vec3{0.f});

    auto cubeRendererWk = cubeActor->getComponentByType<RenderComponent>();
    if (auto cubeRendererComponent = cubeRendererWk.lock())
    {
        cubeRendererComponent->setModel(cubeModel);
    }
}

} // namespace cmx
