#include "cmx_game.h"

#include "cmx_input_action.h"
#include "cmx_input_manager.h"
#include "cmx_viewport_actor.h"

// lib
#include <GLFW/glfw3.h>
#include <glm/ext/scalar_constants.hpp>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <cstdlib>

namespace cmx
{

World *Game::getWorld()
{
    if (!activeWorld)
    {
        spdlog::critical("Game: No active world! There MUST be an active world at all times");
        std::exit(EXIT_FAILURE);
    }
    return activeWorld;
}

void Game::loadEditor()
{
    if (!inputManager)
    {
        spdlog::error("Editor: Missing input manager.");
        return;
    }

    inputManager->addInput("viewport movement", new AxisAction{CMX_KEY_D, CMX_KEY_A, CMX_KEY_W, CMX_KEY_S});
    inputManager->addInput("viewport rotation", new AxisAction{CMX_MOUSE_AXIS_X_RELATIVE, CMX_MOUSE_AXIS_Y_RELATIVE});
    inputManager->addInput("viewport select", new ButtonAction{ButtonAction::Type::PRESSED, {CMX_MOUSE_BUTTON_LEFT}});
    inputManager->addInput("viewport deselect",
                           new ButtonAction{ButtonAction::Type::RELEASED, {CMX_MOUSE_BUTTON_LEFT}});

    std::shared_ptr<ViewportActor> viewportActor = Actor::spawn<ViewportActor>(getWorld(), "Viewport Actor");

    auto cameraWk = viewportActor->getComponentByType<CameraComponent>();
    if (auto cameraComponent = cameraWk.lock())
    {
        getWorld()->setCamera(cameraComponent);
    }
}

} // namespace cmx
