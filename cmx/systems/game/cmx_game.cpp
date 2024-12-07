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
    if (!inputManager)
    {
        spdlog::error("Editor: Missing input manager.");
        return;
    }

    inputManager->addInput("viewport movement", new AxisAction{CMX_KEY_D, CMX_KEY_A, CMX_KEY_W, CMX_KEY_S});
    inputManager->addInput("viewport rotation", new AxisAction{CMX_MOUSE_AXIS_X_RELATIVE, CMX_MOUSE_AXIS_Y_RELATIVE});
    inputManager->addInput("viewport toggle", new ButtonAction{ButtonAction::Type::TOGGLE, {CMX_MOUSE_BUTTON_LEFT}});

    std::shared_ptr<ViewportActor> viewportActor = Actor::spawn<ViewportActor>(getScene(), "ViewportActor");

    std::weak_ptr<ViewportUIComponent> viewportUIWk = viewportActor->getComponentByType<ViewportUIComponent>();
    if (auto viewportUIComponent = viewportUIWk.lock())
    {
        viewportUIComponent->initImGUI(cmxDevice, cmxWindow, cmxRenderer);
    }

    std::weak_ptr<CameraComponent> cameraWk = viewportActor->getComponentByType<CameraComponent>();
    if (auto cameraComponent = cameraWk.lock())
    {
        getScene()->setCamera(cameraComponent);
    }
}

tinyxml2::XMLElement &Game::save(const char *filepath)
{
    tinyxml2::XMLDocument doc;
    // Add declaration
    doc.InsertFirstChild(doc.NewDeclaration());

    // Create the root element
    tinyxml2::XMLElement *root = doc.NewElement("game");
    root->SetAttribute("width", WIDTH);
    root->SetAttribute("height", HEIGHT);
    doc.InsertEndChild(root);

    for (int i = 0; i < scenes.size(); i++)
    {
        scenes.at(i)->save(doc, root);
    }

    if (doc.SaveFile(filepath) != tinyxml2::XML_SUCCESS)
    {
        spdlog::error("FILE SAVING: {0}", doc.ErrorStr());
    };

    return *root;
}

} // namespace cmx
