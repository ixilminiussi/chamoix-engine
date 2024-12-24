#include "cmx_game.h"

// cmx
#include "cmx_camera_component.h"
#include "cmx_device.h"
#include "cmx_input_manager.h"
#include "cmx_render_system.h"
#include "cmx_viewport_actor.h"
#include "cmx_viewport_ui_component.h"

// lib
#include <GLFW/glfw3.h>
#include <glm/ext/scalar_constants.hpp>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <cstdlib>

namespace cmx
{

Game::Game()
{
    _cmxWindow = std::make_unique<CmxWindow>(WIDTH, HEIGHT, "chamoix");

    _inputManager = std::make_shared<InputManager>(*_cmxWindow.get());
    _renderSystem = std::make_shared<RenderSystem>(*_cmxWindow.get());
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

CmxDevice &Game::getDevice()
{
    return *_renderSystem->_cmxDevice.get();
}

void Game::loadEditor()
{
    std::shared_ptr<ViewportActor> viewportActor = Actor::spawn<ViewportActor>(getScene(), "ViewportActor");

    std::weak_ptr<ViewportUIComponent> viewportUIWk = viewportActor->getComponentByType<ViewportUIComponent>();
    if (auto viewportUIComponent = viewportUIWk.lock())
    {
        viewportUIComponent->initInputManager(*_cmxWindow.get());
        viewportUIComponent->initImGUI(_renderSystem.get());
    }

    std::weak_ptr<CameraComponent> cameraWk = viewportActor->getComponentByType<CameraComponent>();
    if (auto cameraComponent = cameraWk.lock())
    {
        getScene()->setCamera(cameraComponent);
    }
}

} // namespace cmx
