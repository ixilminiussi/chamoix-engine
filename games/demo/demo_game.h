#pragma once

// cmx
#include "cmx_game.h"
#include "cmx_world.h"

// lib
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>

// std
#include <cstdlib>

class Demo : public cmx::Game
{
  public:
    static constexpr int WIDTH = 1080;
    static constexpr int HEIGHT = 720;

    Demo() = default;
    ~Demo();
    void load() override;
    void run() override;
    void closeWindow(float dt, int val);

  protected:
    cmx::World mainWorld{"Main", this};
};
