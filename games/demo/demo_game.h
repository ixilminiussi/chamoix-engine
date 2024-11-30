#pragma once

// cmx
#include "cmx_device.h"
#include "cmx_game.h"
#include "cmx_renderer.h"
#include "cmx_world.h"

// lib
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>

// std
#include <cstdlib>
#include <memory>

class Demo : public cmx::Game
{
  public:
    static constexpr int WIDTH = 1080;
    static constexpr int HEIGHT = 720;

    Demo() = default;
    ~Demo();
    void load() override;
    void run() override;

  protected:
    cmx::CmxWindow cmxWindow{WIDTH, HEIGHT, "demo"};
    cmx::CmxDevice cmxDevice{cmxWindow};
    cmx::CmxRenderer cmxRenderer{cmxWindow, cmxDevice};
    std::unique_ptr<VkPipeline> cmxPipeline;
    VkPipelineLayout pipelineLayout;

    cmx::World mainWorld{"Main", this};
};
