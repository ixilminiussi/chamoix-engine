#include "cmx_camera_component.h"

#include "cmx_input_manager.h"
#include "game.h"
#include "viewport_actor.h"

// lib
#include <functional>
#include <spdlog/spdlog.h>

// std
#include <memory>

void ViewportActor::onBegin()
{
    camera = std::make_shared<cmx::CmxCameraComponent>();
    attachComponent(camera);

    getWorld()->getGame()->getInputManager().bindButton(
        "jump", std::bind(&ViewportActor::onJumpInput, this, std::placeholders::_1));
    getWorld()->getGame()->getInputManager().bindAxis(
        "lateral movement",
        std::bind(&ViewportActor::onMovementInput, this, std::placeholders::_1, std::placeholders::_2));
}

void ViewportActor::update(float dt)
{
}

void ViewportActor::onMovementInput(float dt, glm::vec2 movement)
{
    spdlog::info("x: {0}, y: {1}", transform.position.x, transform.position.y);
    transform.position += glm::vec3(movement * 100.f * dt, 0.f);
}

void ViewportActor::onJumpInput(float dt)
{
    spdlog::info("jumping");
}
