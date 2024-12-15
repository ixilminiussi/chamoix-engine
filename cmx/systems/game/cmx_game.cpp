#include "cmx_game.h"

#include "cmx_input_action.h"
#include "cmx_viewport_actor.h"
#include "cmx_viewport_ui_component.h"
#include "tinyxml2.h"

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

Scene *Game::getScene()
{
    if (!activeScene)
    {
        spdlog::critical("Game: No active scene! There MUST be an active scene at all times");
        std::exit(EXIT_FAILURE);
    }
    return activeScene;
}

void Game::loadEditor()
{
    std::shared_ptr<ViewportActor> viewportActor = Actor::spawn<ViewportActor>(getScene(), "ViewportActor");

    std::weak_ptr<ViewportUIComponent> viewportUIWk = viewportActor->getComponentByType<ViewportUIComponent>();
    if (auto viewportUIComponent = viewportUIWk.lock())
    {
        viewportUIComponent->initInputManager(cmxWindow);
        viewportUIComponent->initImGUI(cmxDevice, cmxWindow, cmxRenderer);
    }

    std::weak_ptr<CameraComponent> cameraWk = viewportActor->getComponentByType<CameraComponent>();
    if (auto cameraComponent = cameraWk.lock())
    {
        getScene()->setCamera(cameraComponent);
    }
}

} // namespace cmx
