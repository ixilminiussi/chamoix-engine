#ifndef CMX_GAME
#define CMX_GAME

// cmx
#include "cmx_window.h"

// lib
#include <spdlog/spdlog.h>
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

    virtual void run() {};

    // getters and setters :: begin
    class Scene *getScene();
    void setScene(size_t i);

    class Scene *newScene();

    class InputManager *getInputManager() const
    {
        return _inputManager.get();
    }
    static Window &getWindow();
    // getters and setters :: end

  protected:
    class Scene *_activeScene{nullptr};
    std::vector<Scene *> _scenes;

    static Window _window;

    std::unique_ptr<class InputManager> _inputManager;

    // warning flags
    bool _noCameraFlag{false};
};

} // namespace cmx

#endif
