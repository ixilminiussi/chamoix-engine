#include "descent_game.h"

#include "enemy_ship_actor.h"
#include "gun_component.h"
#include "room_actor.h"
#include "ship_actor.h"
#include "ship_camera_component.h"
#include "wall_actor.h"

// cmx
#include <cmx_actor.h>
#include <cmx_assets_manager.h>
#include <cmx_editor.h>
#include <cmx_input_manager.h>
#include <cmx_register.h>
#include <cmx_render_system.h>
#include <cmx_scene.h>
#include <cmx_window.h>

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
    cmx::Register &cmxRegister = cmx::Register::getInstance();
}

Descent::~Descent()
{
}

void Descent::run()
{
#ifndef NDEBUG
    cmx::Editor *editor = cmx::Editor::getInstance();
#endif
    while (!_window.shouldClose())
    {
        float dt = (float)glfwGetTime();
        glfwSetTime(0.);

#ifndef NDEBUG
        editor->update(dt);
        if (!cmx::Editor::isActive())
        {
#endif
            getInputManager()->pollEvents(dt);
            getScene()->update(dt);
#ifndef NDEBUG
        }
#endif
        getScene()->render();
    }

    getScene()->unload();

    cmx::RenderSystem::getInstance()->closeWindow();
}

void Descent::load()
{
    _inputManager->load();

    _scenes.push_back(&mainScene);
    setScene(0);
}
