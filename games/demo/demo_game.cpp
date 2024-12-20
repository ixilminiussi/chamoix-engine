#include "demo_game.h"

#include "cmx_actor.h"
#include "cmx_assets_manager.h"
#include "cmx_mesh_component.h"
#include "cmx_scene.h"
#include "rotating_actor.h"

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
}

void Demo::run()
{
    while (!cmxWindow.shouldClose())
    {
        float dt = glfwGetTime();
        glfwSetTime(0.);

        getInputManager()->pollEvents(dt);

        getScene()->update(dt);
    }
}

void Demo::closeWindow(float dt, int val)
{
    std::exit(EXIT_SUCCESS);
}

void Demo::load()
{
    inputManager->load();
    renderSystem->initialize();

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
