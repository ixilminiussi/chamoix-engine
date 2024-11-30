#pragma once

#include "cmx_device.h"
#include "cmx_input_action.h"
#include "cmx_input_manager.h"
#include "cmx_pipeline.h"
#include "cmx_renderer.h"
#include "cmx_window.h"
#include "cmx_world.h"

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
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;

    Game();
    ~Game();

    Game(const Game &) = delete;
    Game &operator=(const Game &) = delete;

    void run();

    // getters and setters :: begin
    class World *getWorld()
    {
        if (!activeWorld)
        {
            spdlog::critical("No active world! There MUST be an active world at all times");
            std::exit(EXIT_FAILURE);
        }
        return activeWorld;
    }
    void setWorld(class World *newWorld)
    {
        activeWorld = newWorld;
    }
    CmxInputManager &getInputManager()
    {
        return cmxInputManager;
    }
    // getters and setters :: end

  private:
    virtual void load();

    CmxWindow cmxWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
    CmxDevice cmxDevice{cmxWindow};
    CmxRenderer cmxRenderer{cmxWindow, cmxDevice};
    std::unique_ptr<CmxPipeline> cmxPipeline;
    VkPipelineLayout pipelineLayout;

    class World *activeWorld;

    // game specific
    World mainWorld{"Main", this};
    CmxInputManager cmxInputManager{
        cmxWindow,
        {
            {
                "jump",
                new CmxButtonAction{CmxButtonAction::Type::PRESSED, {CMX_KEY_SPACE, CMX_KEY_A}},
            },
            {
                "exit",
                new CmxButtonAction{CmxButtonAction::Type::PRESSED, {CMX_KEY_ESCAPE}},
            },
            {"lateral movement", new CmxAxisAction{CMX_KEY_D, CMX_KEY_A, CMX_KEY_W, CMX_KEY_S}},
            {"view rotation", new CmxAxisAction{CMX_MOUSE_AXIS_X_RELATIVE, CMX_MOUSE_AXIS_Y_RELATIVE}},
            {"select", new CmxButtonAction{CmxButtonAction::Type::PRESSED, {CMX_MOUSE_BUTTON_LEFT}}},
            {"deselect", new CmxButtonAction{CmxButtonAction::Type::RELEASED, {CMX_MOUSE_BUTTON_LEFT}}},
        }};
};

} // namespace cmx
