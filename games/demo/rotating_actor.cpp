#include "rotating_actor.h"

// cmx
#include "cmx_game.h"
#include "cmx_render_component.h"

// lib
#include <glm/ext/scalar_constants.hpp>

void RotatingActor::onBegin()
{
    transform.position = {0.f, 0.f, 5.f};
    transform.scale = {.5f, .5f, .5f};
    transform.rotation = {0.f, 0.f, 0.f};

    auto inputManager = getWorld()->getGame()->getInputManager();
    if (inputManager)
    {
        inputManager->bindButton("slowdown toggle", &RotatingActor::slowdownToggle, this);
    }

    auto renderComponent = std::make_shared<cmx::RenderComponent>();
    attachComponent(renderComponent);
}

void RotatingActor::update(float dt)
{
    transform.rotation.y += rotationSpeed * dt * glm::pi<float>();
}

void RotatingActor::slowdownToggle(float dt, int val)
{
    if (val == 1)
    {
        rotationSpeed = rotationSpeedSlow;
    }
    else
    {
        rotationSpeed = rotationSpeedFast;
    }
}
