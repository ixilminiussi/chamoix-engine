#pragma once

// cmx
#include "cmx_game.h"
#include "cmx_scene.h"
#include "tinyxml2.h"

// lib
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>

// std
#include <cstdlib>

class Demo : public cmx::Game
{
  public:
    Demo() = default;
    ~Demo();
    void load() override;
    void run() override;
    void closeWindow(float dt, int val);

    tinyxml2::XMLElement &save(const char *filepath) override;

  protected:
    cmx::Scene mainScene{"Main", this};
};
