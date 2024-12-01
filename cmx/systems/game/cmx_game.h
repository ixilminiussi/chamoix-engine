#pragma once

// cmx
#include "cmx_descriptors.h"
#include "cmx_device.h"
#include "cmx_input_action.h"
#include "cmx_input_manager.h"
#include "cmx_renderer.h"
#include "cmx_window.h"

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
    Game() = default;
    ~Game() = default;

    Game(const Game &) = delete;
    Game &operator=(const Game &) = delete;

    virtual void load() {};
    void loadEditor();
    virtual void run() {};

    // getters and setters :: begin
    class World *getWorld();
    void setWorld(class World *world)
    {
        activeWorld = world;
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
    class World *activeWorld{};

    CmxWindow cmxWindow{1080, 720, "demo"};
    CmxDevice cmxDevice{cmxWindow};
    CmxRenderer cmxRenderer{cmxWindow, cmxDevice};
    std::unique_ptr<VkPipeline> cmxPipeline;
    VkPipelineLayout pipelineLayout;

    std::unique_ptr<CmxDescriptorPool> globalPool{};

    std::shared_ptr<InputManager> inputManager;
};

} // namespace cmx
