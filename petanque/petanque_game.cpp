#include "petanque_game.h"

#include "dynamic_body_actor.h"
#include "first_person_actor.h"
#include "rigid_body_actor.h"
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

Petanque::Petanque()
{
    cmx::Register &cmxRegister = cmx::Register::getInstance();
}

Petanque::~Petanque()
{
}

void Petanque::run()
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

void Petanque::load()
{
    _inputManager->load();

    _scenes.emplace_back(new cmx::Scene("scenes/field.xml", this, "field"));
    setScene(0);
}
