#pragma once

// cmx
#include "cmx_descriptors.h"
#include "cmx_device.h"
#include "cmx_input_action.h"
#include "cmx_input_manager.h"
#include "cmx_renderer.h"
#include "cmx_window.h"
#include "tinyxml2.h"

// lib
#include <unordered_map>
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
    virtual tinyxml2::XMLElement &save(const char *filepath);

    virtual void run() {};

    // getters and setters :: begin
    class Scene *getScene();
    void setScene(int i)
    {
        activeScene = scenes.at(i);
    }
    void createInputManager(CmxWindow &window, const std::unordered_map<std::string, InputAction *> &inputDictionary)
    {
        inputManager = std::make_shared<InputManager>(window, inputDictionary);
    }
    std::shared_ptr<InputManager> getInputManager()
    {
        return inputManager;
    }
    // getters and setters :: end

  protected:
    class Scene *activeScene{};
    std::vector<Scene *> scenes;

    CmxWindow cmxWindow{WIDTH, HEIGHT, "demo"};
    CmxDevice cmxDevice{cmxWindow};
    CmxRenderer cmxRenderer{cmxWindow, cmxDevice};
    std::unique_ptr<VkPipeline> cmxPipeline;
    VkPipelineLayout pipelineLayout;

    std::unique_ptr<CmxDescriptorPool> globalPool{};

    std::shared_ptr<InputManager> inputManager;

    // warning flags
    bool noCameraFlag{false};
};

inline int Game::WIDTH = 1600;
inline int Game::HEIGHT = 1200;

} // namespace cmx
