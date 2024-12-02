#include "cmx_viewport_actor.h"

// cmx
#include "cmx_camera_component.h"
#include "cmx_game.h"
#include "cmx_input_manager.h"
#include "cmx_viewport_ui_component.h"

// lib
#include <glm/common.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/constants.hpp>
#include <spdlog/spdlog.h>

// std
#include <memory>

namespace cmx
{

void ViewportActor::onBegin()
{
    transform.position = {0.f, 0.f, 0.f};
    transform.scale = {1.f, 1.f, 1.f};
    transform.rotation = {0.f, 0.f, 1.f};

    camera = std::make_shared<CameraComponent>();
    attachComponent(camera);
    attachComponent(std::make_shared<ViewportUIComponent>(moveSpeed, mouseSensitivity));

    std::shared_ptr<InputManager> inputManager = getWorld()->getGame()->getInputManager();

    inputManager->bindAxis("viewport movement", &ViewportActor::onMovementInput, this);
    inputManager->bindAxis("viewport rotation", &ViewportActor::onMouseMovement, this);
    inputManager->bindButton("viewport toggle", &ViewportActor::select, this);
}

void ViewportActor::update(float dt)
{
}

void ViewportActor::onMovementInput(float dt, glm::vec2 movement)
{
    if (!selected)
        return;

    if (glm::length(movement) <= glm::epsilon<float>())
        return;

    movement *= moveSpeed;

    transform.position += transform.forward() * movement.y * dt;
    transform.position += transform.right() * movement.x * dt;
}

void ViewportActor::onJumpInput(float dt)
{
    spdlog::info("jumping");
}

void ViewportActor::onMouseMovement(float dt, glm::vec2 mousePosition)
{
    if (!selected)
        return;

    transform.rotation.y += mouseSensitivity * mousePosition.x * dt;
    transform.rotation.x += mouseSensitivity * mousePosition.y * dt;

    // limit pitch value to +/- 85 degrees
    transform.rotation.x = glm::clamp(transform.rotation.x, -1.5f, 1.5f);
    transform.rotation.y = glm::mod(transform.rotation.y, glm::two_pi<float>());
}

void ViewportActor::select(float dt, int val)
{
    if (val == 1)
    {
        selected = true;
        getWorld()->getGame()->getInputManager()->setMouseCapture(true);
    }
    else
    {
        selected = false;
        getWorld()->getGame()->getInputManager()->setMouseCapture(false);
    }
}

} // namespace cmx
