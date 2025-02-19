#ifndef CMX_GAME
#define CMX_GAME

// cmx
#include "cmx_window.h"

// lib
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>

// std
#include <cstdlib>
#include <map>
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
    void setScene(int i);

    class Scene *newScene();

    class InputManager *getInputManager() const
    {
        return _inputManager.get();
    }
    std::map<uint8_t, std::shared_ptr<class RenderSystem>> &getRenderSystems()
    {
        return _renderSystems;
    }
    static Window &getWindow();
    // getters and setters :: end

  protected:
    class Scene *_activeScene{nullptr};
    std::vector<Scene *> _scenes;

    static Window _window;

    std::unique_ptr<class InputManager> _inputManager;
    std::map<uint8_t, std::shared_ptr<class RenderSystem>> _renderSystems;

    // warning flags
    bool _noCameraFlag{false};
};

} // namespace cmx

#endif
