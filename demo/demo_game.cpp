#include "demo_game.h"

#include "dynamic_body_actor.h"
#include "first_person_actor.h"
#include "rigid_body_actor.h"
#include "rotating_actor.h"
#include "static_body_actor.h"

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
#include <memory>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <algorithm>
#include <cstdlib>

Demo::Demo()
{
    cmx::Register &cmxRegister = cmx::Register::getInstance();
}

Demo::~Demo()
{
}

void Demo::run()
{
#ifndef NDEBUG
    cmx::Editor *editor = cmx::Editor::getInstance();
#endif
    while (!_window.shouldClose())
    {
        float dt = std::min(glfwGetTime(), 0.2);
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

void Demo::load()
{
    _inputManager->load();

    _scenes.push_back(&mainScene);
    setScene(0);
}
