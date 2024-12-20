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
        if (activeScene)
        {
            activeScene->unload();
        }
        try
        {
            activeScene = scenes.at(i);
            activeScene->load();
        }
        catch (const std::out_of_range &e)
        {
            spdlog::error("Scene: no scene at index {0}", i);
        }
    }
    std::shared_ptr<InputManager> getInputManager() const
    {
        return inputManager;
    }
    std::shared_ptr<RenderSystem> getRenderSystem() const
    {
        return renderSystem;
    }
    CmxDevice &getDevice()
    {
        return *renderSystem->cmxDevice;
    }
    // getters and setters :: end

  protected:
    class Scene *activeScene;
    std::vector<Scene *> scenes;

    CmxWindow cmxWindow{WIDTH, HEIGHT, "demo"};

    std::shared_ptr<InputManager> inputManager = std::make_shared<InputManager>(cmxWindow);
    std::shared_ptr<RenderSystem> renderSystem = std::make_shared<RenderSystem>(cmxWindow);

    // warning flags
    bool noCameraFlag{false};
};

inline int Game::WIDTH = 1600;
inline int Game::HEIGHT = 1200;

} // namespace cmx
