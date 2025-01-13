#include "descent_game.h"

#include "cmx/cmx_editor.h"
#include "room_actor.h"
#include "wall_actor.h"

// cmx
#include <cmx/cmx_actor.h>
#include <cmx/cmx_assets_manager.h>
#include <cmx/cmx_billboard_render_system.h>
#include <cmx/cmx_edge_render_system.h>
#include <cmx/cmx_input_manager.h>
#include <cmx/cmx_register.h>
#include <cmx/cmx_render_system.h>
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

Descent::Descent()
{
    cmx::Register *cmxRegister = cmx::Register::getInstance();

    cmxRegister->addActor("WallActor", [](cmx::Scene *scene, const std::string &name) {
        return cmx::Actor::spawn<WallActor>(scene, name);
    });

    cmxRegister->addActor("RoomActor", [](cmx::Scene *scene, const std::string &name) {
        return cmx::Actor::spawn<RoomActor>(scene, name);
    });
}

Descent::~Descent()
{
}

void Descent::run()
{
#ifndef NDEBUG
    cmx::CmxEditor *editor = cmx::CmxEditor::getInstance();
#endif
    while (!_cmxWindow.shouldClose())
    {
        float dt = glfwGetTime();
        glfwSetTime(0.);

#ifndef NDEBUG
        editor->update(dt);
#endif
        getScene()->update(dt);
    }
}

void Descent::closeWindow(float dt, int val)
{
    std::exit(EXIT_SUCCESS);
}

void Descent::load()
{
    _inputManager->load();
    _renderSystems[SHADED_RENDER_SYSTEM]->initialize();
    _renderSystems[BILLBOARD_RENDER_SYSTEM]->initialize();
    _renderSystems[EDGE_RENDER_SYSTEM]->initialize();

    _scenes.push_back(&mainScene);
    setScene(0);

    getScene()->getAssetsManager()->addTexture("assets/textures/bricks.png", "bricks");

    getInputManager()->bindButton("exit", &Descent::closeWindow, this);
}
