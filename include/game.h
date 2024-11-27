#pragma once

#include "cmx_device.h"
#include "cmx_pipeline.h"
#include "cmx_swap_chain.h"
#include "cmx_window.h"
#include "cmx_world.h"

// std
#include <memory>
#include <vector>
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
    void load();
    void render(VkCommandBuffer commandBuffer);

    void createPipelineLayout();
    void createPipeline();
    void createCommandBuffers();
    void freeCommandBuffers();
    void drawFrame();
    void recreateSwapChain();
    void recordCommandBuffer(int imageIndex);

    CmxWindow cmxWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
    CmxDevice cmxDevice{cmxWindow};
    std::unique_ptr<CmxSwapChain> cmxSwapChain;
    std::unique_ptr<CmxPipeline> cmxPipeline;
    VkPipelineLayout pipelineLayout;
    std::vector<VkCommandBuffer> commandBuffers;

    World mainWorld{"Main"};
    class World *activeWorld;
};
} // namespace cmx
