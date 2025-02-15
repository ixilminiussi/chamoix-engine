#ifndef PETANQUE_GAME
#define PETANQUE_GAME

// cmx
#include "cmx/cmx_game.h"
#include "cmx/cmx_scene.h"

// lib
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>

// std
#include <cstdlib>

class Petanque : public cmx::Game
{
  public:
    Petanque();
    ~Petanque();
    void load() override;
    void run() override;
    void closeWindow(float dt, int val);
};

#endif
