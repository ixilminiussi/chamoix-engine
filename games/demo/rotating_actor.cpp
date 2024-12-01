#include "rotating_actor.h"

// cmx
#include "cmx_game.h"
#include "cmx_render_component.h"

// lib
#include <functional>
#include <glm/ext/scalar_constants.hpp>

void RotatingActor::onBegin()
{
    transform.position = {0.f, 0.f, 5.f};
    transform.scale = {.5f, .5f, .5f};
    transform.rotation = {0.f, 0.f, 0.f};

    auto inputManager = getWorld()->getGame()->getInputManager();
    if (inputManager)
    {
        inputManager->bindButton("slowdown on", &RotatingActor::slowdownOn, this);
        inputManager->bindButton("slowdown off", &RotatingActor::slowdownOff, this);
    }

    auto renderComponent = std::make_shared<cmx::RenderComponent>();
    attachComponent(renderComponent);
}

void RotatingActor::update(float dt)
{
    transform.rotation.y += rotationSpeed * dt * glm::pi<float>();
}

void RotatingActor::slowdownOn(float dt)
{
    rotationSpeed = rotationSpeedSlow;
}

void RotatingActor::slowdownOff(float dt)
{
    rotationSpeed = rotationSpeedFast;
}
