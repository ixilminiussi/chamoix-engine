#include "descent_game.h"

#include "enemy_ship_actor.h"
#include "gun_component.h"
#include "room_actor.h"
#include "ship_actor.h"
#include "ship_camera_component.h"
#include "wall_actor.h"

// cmx
#include <cmx/cmx_actor.h>
#include <cmx/cmx_assets_manager.h>
#include <cmx/cmx_billboard_render_system.h>
#include <cmx/cmx_edge_render_system.h>
#include <cmx/cmx_editor.h>
#include <cmx/cmx_hud_render_system.h>
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
    cmx::Register &cmxRegister = cmx::Register::getInstance();

    cmxRegister.addActor("WallActor", [](cmx::Scene *scene, const std::string &name) {
        return cmx::Actor::spawn<WallActor>(scene, name);
    });
    cmxRegister.addActor("RoomActor", [](cmx::Scene *scene, const std::string &name) {
        return cmx::Actor::spawn<RoomActor>(scene, name);
    });
    cmxRegister.addActor("ShipActor", [](cmx::Scene *scene, const std::string &name) {
        return cmx::Actor::spawn<ShipActor>(scene, name);
    });
    cmxRegister.addActor("EnemyShipActor", [](cmx::Scene *scene, const std::string &name) {
        return cmx::Actor::spawn<EnemyShipActor>(scene, name);
    });
    cmxRegister.addComponent("ShipCameraComponent", []() { return std::make_shared<ShipCameraComponent>(); });
    cmxRegister.addComponent("GunComponent", []() { return std::make_shared<GunComponent>(); });

    _renderSystems[SHADED_RENDER_SYSTEM] = std::make_shared<cmx::ShadedRenderSystem>();
    _renderSystems[BILLBOARD_RENDER_SYSTEM] = std::make_shared<cmx::BillboardRenderSystem>();
    _renderSystems[EDGE_RENDER_SYSTEM] = std::make_shared<cmx::EdgeRenderSystem>();
    _renderSystems[HUD_RENDER_SYSTEM] = std::make_shared<cmx::HudRenderSystem>();

    _renderSystems[SHADED_RENDER_SYSTEM]->initialize();
    _renderSystems[BILLBOARD_RENDER_SYSTEM]->initialize();
    _renderSystems[EDGE_RENDER_SYSTEM]->initialize();
    _renderSystems[HUD_RENDER_SYSTEM]->initialize();
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
        float dt = glfwGetTime();
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

    for (const auto &[key, renderSystem] : _renderSystems)
    {
        renderSystem->free();
    }
    _renderSystems.clear();
    cmx::RenderSystem::closeWindow();
}

void Descent::load()
{
    _inputManager->load();

    _scenes.push_back(&mainScene);
    setScene(0);

    // getScene()->getAssetsManager()->addTexture("assets/textures/bricks.png", "bricks");
}
