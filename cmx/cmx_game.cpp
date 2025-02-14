#include "cmx_game.h"

// cmx
#include "cmx_actor.h"
#include "cmx_input_manager.h"
#include "cmx_scene.h"
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

Window Game::_window = Window{WIDTH, HEIGHT, "chamoix"};

Game::Game()
{
    _inputManager = std::make_unique<InputManager>(_window);

#ifdef NDEBUG
    auto my_logger = spdlog::default_logger();
    my_logger->set_level(spdlog::level::off);
    spdlog::set_default_logger(my_logger);
#endif
}

Game::~Game()
{
}

void Game::setScene(int i)
{
    if (_activeScene != nullptr)
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

Scene *Game::getScene()
{
    if (!_activeScene)
    {
        spdlog::critical("Game: No active scene! There MUST be an active scene at all times");
        std::exit(EXIT_FAILURE);
    }
    return _activeScene;
}

Window &Game::getWindow()
{
    return _window;
}

} // namespace cmx
