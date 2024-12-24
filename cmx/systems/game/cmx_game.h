#pragma once

// lib
#include <spdlog/spdlog.h>
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

    Game();
    ~Game();

    Game(const Game &) = delete;
    Game &operator=(const Game &) = delete;

    virtual void load() {};
    void loadEditor();

    virtual void run() {};

    // getters and setters :: begin
    class Scene *getScene();
    void setScene(int i);

    std::shared_ptr<class InputManager> getInputManager() const
    {
        return _inputManager;
    }
    std::shared_ptr<class RenderSystem> getRenderSystem() const
    {
        return _renderSystem;
    }
    class CmxDevice &getDevice();
    // getters and setters :: end

  protected:
    class Scene *_activeScene;
    std::vector<Scene *> _scenes;

    std::unique_ptr<class CmxWindow> _cmxWindow;

    std::shared_ptr<InputManager> _inputManager;
    std::shared_ptr<RenderSystem> _renderSystem;

    // warning flags
    bool __noCameraFlag{false};
};

inline int Game::WIDTH = 1600;
inline int Game::HEIGHT = 1200;

} // namespace cmx
