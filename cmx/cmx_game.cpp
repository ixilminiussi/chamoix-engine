#include "cmx_game.h"

// cmx
#include "cmx_billboard_render_system.h"
#include "cmx_camera_component.h"
#include "cmx_device.h"
#include "cmx_edge_render_system.h"
#include "cmx_input_manager.h"
#include "cmx_render_system.h"
#include "cmx_shaded_render_system.h"
#include "cmx_viewport_actor.h"
#include "cmx_viewport_ui_component.h"
#include "cmx_window.h"

// lib
#include <GLFW/glfw3.h>
#include <glm/ext/scalar_constants.hpp>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <cstdlib>

namespace cmx
{

int Game::WIDTH = 1600;
int Game::HEIGHT = 1200;

CmxWindow Game::_cmxWindow = CmxWindow{WIDTH, HEIGHT, "chamoix"};

Game::Game()
{
    _renderSystems[SHADED_RENDER_SYSTEM] = std::make_shared<ShadedRenderSystem>();
    _renderSystems[BILLBOARD_RENDER_SYSTEM] = std::make_shared<BillboardRenderSystem>();
#ifndef NDEBUG
    _renderSystems[EDGE_RENDER_SYSTEM] = std::make_shared<EdgeRenderSystem>();
#endif

    _inputManager = std::make_shared<InputManager>(_cmxWindow);
}

Game::~Game()
{
}

void Game::setScene(int i)
{
    {
        if (_activeScene)
        {
            _activeScene->unload();
        }
        try
        {
            _activeScene = _scenes.at(i);
            _activeScene->load();
        }
        catch (const std::out_of_range &e)
        {
            spdlog::error("Scene: no scene at index {0}", i);
        }
    }
}

Scene *Game::getScene()
{
    if (!_activeScene)
    {
        spdlog::critical("Game: No active scene! There MUST be an active scene at all times");
        std::exit(EXIT_FAILURE);
    }
    return _activeScene;
}

CmxWindow &Game::getWindow()
{
    return _cmxWindow;
}

std::shared_ptr<CmxDevice> Game::getDevice()
{
    try
    {
        return RenderSystem::getDevice();
    }
    catch (std::out_of_range e)
    {
        spdlog::info("Game: cannot call getDevice() before having ran RenderSystem::build(CmxWindow&)");
        std::exit(EXIT_FAILURE);
    }
}

void Game::loadEditor()
{
    std::shared_ptr<ViewportActor> viewportActor = Actor::spawn<ViewportActor>(getScene(), "ViewportActor");

    std::weak_ptr<ViewportUIComponent> viewportUIWk = viewportActor->getComponentByType<ViewportUIComponent>();
    if (auto viewportUIComponent = viewportUIWk.lock())
    {
        viewportUIComponent->initInputManager(_cmxWindow);
        viewportUIComponent->initImGUI();
    }

    std::weak_ptr<CameraComponent> cameraWk = viewportActor->getCameraComponent();
    if (auto cameraComponent = cameraWk.lock())
    {
        getScene()->setCamera(cameraComponent->getCamera());
    }
}

} // namespace cmx
