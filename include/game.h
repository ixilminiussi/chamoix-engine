#pragma once

#include "cmx_device.h"
#include "cmx_pipeline.h"
#include "cmx_renderer.h"
#include "cmx_window.h"
#include "cmx_world.h"

// std
#include <memory>
#include <vulkan/vulkan_core.h>

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
        return activeWorld;
    }
    void setWorld(class World *newWorld)
    {
        activeWorld = newWorld;
    }
    // getters and setters :: end

  private:
    virtual void load();

    CmxWindow cmxWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
    CmxDevice cmxDevice{cmxWindow};
    CmxRenderer cmxRenderer{cmxWindow, cmxDevice};
    std::unique_ptr<CmxPipeline> cmxPipeline;
    VkPipelineLayout pipelineLayout;

    World mainWorld{"Main"};
    class World *activeWorld;
};
} // namespace cmx
