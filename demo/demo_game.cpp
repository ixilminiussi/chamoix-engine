#include "demo_game.h"

#include "cmx/cmx_render_system.h"
#include "dynamic_body_actor.h"
#include "rotating_actor.h"
#include "static_body_actor.h"

// cmx
#include <cmx/cmx_actor.h>
#include <cmx/cmx_assets_manager.h>
#include <cmx/cmx_billboard_render_system.h>
#include <cmx/cmx_input_manager.h>
#include <cmx/cmx_register.h>
#include <cmx/cmx_scene.h>
#include <cmx/cmx_shaded_render_system.h>
#include <cmx/cmx_window.h>

// lib
#include <GLFW/glfw3.h>
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
#include <cstdlib>
#include <memory>

Demo::Demo()
{
    cmx::Register *cmxRegister = cmx::Register::getInstance();

    cmxRegister->addActor("RotatingActor", [](cmx::Scene *scene, const std::string &name) {
        return cmx::Actor::spawn<RotatingActor>(scene, name);
    });
    cmxRegister->addActor("StaticBodyActor", [](cmx::Scene *scene, const std::string &name) {
        return cmx::Actor::spawn<StaticBodyActor>(scene, name);
    });
    cmxRegister->addActor("DynamicBodyActor", [](cmx::Scene *scene, const std::string &name) {
        return cmx::Actor::spawn<DynamicBodyActor>(scene, name);
    });
}

Demo::~Demo()
{
}

void Demo::run()
{
    while (!_cmxWindow.shouldClose())
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
    _renderSystems[SHADED_RENDER_SYSTEM]->initialize();
    _renderSystems[BILLBOARD_RENDER_SYSTEM]->initialize();
#ifndef NDEBUG
    _renderSystems[EDGE_RENDER_SYSTEM]->initialize();
#endif

    _scenes.push_back(&mainScene);
    setScene(0);

    getScene()->getAssetsManager()->addTexture("assets/textures/bricks.png", "bricks");

    getInputManager()->bindButton("exit", &Demo::closeWindow, this);
}
