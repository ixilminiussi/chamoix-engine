#include "demo_game.h"

#include "cmx_input_manager.h"
#include "cmx_render_system.h"
#include "cmx_scene.h"
#include "cmx_window.h"

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

Demo::Demo()
{
}

Demo::~Demo()
{
}

void Demo::run()
{
    while (!_cmxWindow->shouldClose())
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
    _inputManager->load();
    _renderSystem->initialize();

    _scenes.push_back(&mainScene);
    setScene(0);

    getInputManager()->bindButton("exit", &Demo::closeWindow, this);
}
