#include "demo_game.h"

// cmx
#include <algorithm>
#include <cmx/cmx_actor.h>
#include <cmx/cmx_assets_manager.h>
#include <cmx/cmx_billboard_render_system.h>
#include <cmx/cmx_edge_render_system.h>
#include <cmx/cmx_editor.h>
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
#include <memory>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <cstdlib>

Demo::Demo()
{
    cmx::Register &cmxRegister = cmx::Register::getInstance();

    _renderSystems[SHADED_RENDER_SYSTEM] = std::make_shared<cmx::ShadedRenderSystem>();
    _renderSystems[BILLBOARD_RENDER_SYSTEM] = std::make_shared<cmx::BillboardRenderSystem>();
    _renderSystems[EDGE_RENDER_SYSTEM] = std::make_shared<cmx::EdgeRenderSystem>();

    _renderSystems[SHADED_RENDER_SYSTEM]->initialize();
    _renderSystems[BILLBOARD_RENDER_SYSTEM]->initialize();
    _renderSystems[EDGE_RENDER_SYSTEM]->initialize();
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

    for (auto &[key, renderSystem] : _renderSystems)
    {
        renderSystem->free();
    }
    _renderSystems.clear();
    cmx::RenderSystem::closeWindow();
}

void Demo::load()
{
    _inputManager->load();

    _scenes.push_back(&mainScene);
    setScene(0);
}
