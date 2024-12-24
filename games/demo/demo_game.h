#pragma once

// cmx
#include "cmx_game.h"
#include "cmx_scene.h"

// lib
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>

// std
#include <cstdlib>

class Demo : public cmx::Game
{
  public:
    Demo();
    ~Demo();
    void load() override;
    void run() override;
    void closeWindow(float dt, int val);

  protected:
    cmx::Scene mainScene{"scenes/demo-scene.xml", this, "first scene"};
};
