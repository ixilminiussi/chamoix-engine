#include "cmx_camera_component.h"

#include "cmx_input_manager.h"
#include "game.h"
#include "viewport_actor.h"

// lib
#include <functional>
#include <glm/common.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/constants.hpp>
#include <limits>
#include <spdlog/spdlog.h>

// std
#include <memory>

void ViewportActor::onBegin()
{
    transform.position = {0.f, 0.f, 0.f};
    transform.scale = {1.f, 1.f, 1.f};
    transform.rotation = {0.f, 0.f, 1.f};

    camera = std::make_shared<cmx::CameraComponent>();
    attachComponent(camera);

    cmx::CmxInputManager &inputManager = getWorld()->getGame()->getInputManager();

    inputManager.bindButton("jump", std::bind(&ViewportActor::onJumpInput, this, std::placeholders::_1));
    inputManager.bindAxis("lateral movement", std::bind(&ViewportActor::onMovementInput, this, std::placeholders::_1,
                                                        std::placeholders::_2));
    inputManager.bindAxis("view rotation", std::bind(&ViewportActor::onMouseMovement, this, std::placeholders::_1,
                                                     std::placeholders::_2));
    inputManager.bindButton("select", std::bind(&ViewportActor::select, this, std::placeholders::_1));
    inputManager.bindButton("deselect", std::bind(&ViewportActor::deselect, this, std::placeholders::_1));
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

void ViewportActor::select(float dt)
{
    selected = true;
    getWorld()->getGame()->getInputManager().setMouseCapture(true);
}

void ViewportActor::deselect(float dt)
{
    selected = false;
    getWorld()->getGame()->getInputManager().setMouseCapture(false);
}
