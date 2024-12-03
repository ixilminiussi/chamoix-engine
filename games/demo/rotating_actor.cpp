#include "rotating_actor.h"

// cmx
#include "cmx_game.h"
#include "cmx_render_component.h"
#include "imgui.h"

// lib
#include <glm/ext/scalar_constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

void RotatingActor::onBegin()
{
    transform.position = {0.f, 0.f, -5.f};
    transform.scale = {.5f, .5f, .5f};

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
    transform.rotation = glm::normalize(glm::angleAxis(rotationSpeed * dt * 1.0f, transform.up()) * transform.rotation);
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

void RotatingActor::renderSettings()
{
    ImGui::SeparatorText("Rotation");
    ImGui::DragFloat("Slow Speed", &rotationSpeedSlow, 0.01f, -10.0f, 10.0f);
    ImGui::DragFloat("Fast Speed", &rotationSpeedFast, 0.01f, -10.0f, 10.0f);

    Actor::renderSettings();
}
