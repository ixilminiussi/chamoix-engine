#include "cmx_game.h"

// cmx
#include "cmx_actor.h"
#include "cmx_input_manager.h"
#include "cmx_scene.h"
#include "cmx_sink.h"
#include "cmx_window.h"

// lib
#include <GLFW/glfw3.h>
#include <exception>
#include <glm/ext/scalar_constants.hpp>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
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

void Game::setScene(size_t i)
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
    catch (const std::out_of_range &)
    {
        spdlog::error("Scene: no scene at index {0}", i);
    }
}

Scene *Game::newScene()
{
    srand(static_cast<unsigned>(time(0)));
    int randomNumber = rand();

    std::string randomPath = std::string(".temp/new_" + std::to_string(randomNumber) + ".xml");

    _scenes.push_back(new Scene(randomPath, this, "temp"));

    size_t index = _scenes.size() - 1;
    setScene(index);
    return _scenes.at(index);
}

Scene *Game::getScene()
{
    if (!_activeScene)
    {
        throw std::runtime_error("Game: No active scene! There MUST be an active scene at all times");
    }
    return _activeScene;
}

Window &Game::getWindow()
{
    return _window;
}

} // namespace cmx
