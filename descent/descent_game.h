#ifndef DESCENT_GAME
#define DESCENT_GAME

// cmx
#include <cmx_game.h>
#include <cmx_scene.h>

// lib
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>

// std
#include <cstdlib>

class Descent : public cmx::Game
{
  public:
    Descent();
    ~Descent();
    void load() override;
    void run() override;
    void closeWindow(float dt, int val);

  protected:
    cmx::Scene mainScene{"scenes/gym-scene.xml", this, "gym"};

    bool _playing = false;
};

#endif
