#include "game.h"

#include "cmx_actor.h"
#include "cmx_default_render_system.h"
#include "cmx_model.h"
#include "cmx_pipeline.h"
#include "cmx_render_component.h"
#include "cmx_world.h"
#include "triangle_actor.h"

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

struct SimplePushConstantData
{
    glm::mat2 transform{1.f};
    glm::vec2 offset;
    alignas(16) glm::vec3 color;
};

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
        glfwPollEvents();
        if (!getWorld())
        {
            spdlog::critical("Game requires does not have active world instance");
            std::exit(EXIT_FAILURE);
        }
        getWorld()->updateActors(1);
        getWorld()->updateComponents(1);
        if (auto commandBuffer = cmxRenderer.beginFrame())
        {
            cmxRenderer.beginSwapChainRenderPass(commandBuffer);

            std::vector<std::weak_ptr<Component>> &components = getWorld()->getAllComponents();
            defaultRenderSystem.render(commandBuffer, components);
            cmxRenderer.endSwapChainRenderPass(commandBuffer);
            cmxRenderer.endFrame();
        }
    }

    vkDeviceWaitIdle(cmxDevice.device());
}

void Game::load()
{
    std::vector<CmxModel::Vertex> vertices = {
        {{0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}}, {{0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}}, {{-0.5, 0.5}, {0.0f, 0.0f, 1.0f}}};

    auto cmxModel = std::make_shared<CmxModel>(cmxDevice, vertices);

    setWorld(&mainWorld);

    auto actor = Actor::spawn<TriangleActor>(&mainWorld, "triangle1");

    std::weak_ptr<RenderComponent> renderComponent = actor->getComponentByType<RenderComponent>();
    if (auto shared = renderComponent.lock())
    {
        shared->setModel(cmxModel);
    }
}

} // namespace cmx
