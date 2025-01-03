#ifndef CMX_GAME
#define CMX_GAME

// cmx
#include "cmx_window.h"

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
    auto &getRenderSystems()
    {
        return _renderSystems;
    }
    std::shared_ptr<class CmxDevice> getDevice();
    static CmxWindow &getWindow();
    // getters and setters :: end

  protected:
    class Scene *_activeScene;
    std::vector<Scene *> _scenes;

    static CmxWindow _cmxWindow;

    std::shared_ptr<class InputManager> _inputManager;
    std::unordered_map<uint8_t, std::shared_ptr<class RenderSystem>> _renderSystems;

    // warning flags
    bool __noCameraFlag{false};
};

} // namespace cmx

#endif
