#pragma once

// cmx
#include "cmx_assets_manager.h"
#include "cmx_device.h"
#include "cmx_input_manager.h"
#include "cmx_render_system.h"
#include "cmx_scene.h"
#include "cmx_window.h"

// lib
#include <stdexcept>
#include <vulkan/vulkan_core.h>

// std
#include <cstdlib>
#include <memory>

namespace cmx
{

class Game
{
  public:
    static int WIDTH;
    static int HEIGHT;

    Game() = default;
    ~Game() = default;

    Game(const Game &) = delete;
    Game &operator=(const Game &) = delete;

    virtual void load() {};
    void loadEditor();

    virtual void run() {};

    // getters and setters :: begin
    class Scene *getScene();

    void setScene(int i)
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
    std::shared_ptr<InputManager> getInputManager() const
    {
        return _inputManager;
    }
    std::shared_ptr<RenderSystem> getRenderSystem() const
    {
        return _renderSystem;
    }
    CmxDevice &getDevice()
    {
        return *_renderSystem->_cmxDevice;
    }
    // getters and setters :: end

  protected:
    class Scene *_activeScene;
    std::vector<Scene *> _scenes;

    CmxWindow _cmxWindow{WIDTH, HEIGHT, "demo"};

    std::shared_ptr<InputManager> _inputManager = std::make_shared<InputManager>(_cmxWindow);
    std::shared_ptr<RenderSystem> _renderSystem = std::make_shared<RenderSystem>(_cmxWindow);

    // warning flags
    bool __noCameraFlag{false};
};

inline int Game::WIDTH = 1600;
inline int Game::HEIGHT = 1200;

} // namespace cmx
