#include "rotating_actor.h"

// cmx
#include "cmx_game.h"
#include "cmx_input_manager.h"
#include "cmx_mesh_component.h"

// lib
#include "imgui.h"
#include <glm/ext/scalar_constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

void RotatingActor::onBegin()
{
    cmx::Actor::onBegin();

    cmx::InputManager *inputManager = getScene()->getGame()->getInputManager();
    if (inputManager)
    {
        inputManager->bindButton("slowdown toggle", &RotatingActor::slowdownToggle, this);
    }
}

void RotatingActor::update(float dt)
{
    _transform.rotation = glm::quat({0.f, rotationSpeed * dt * 1.0f, 0.f}) * _transform.rotation;
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

void RotatingActor::editor()
{
    if (ImGui::CollapsingHeader("Rotation"))
    {
        ImGui::DragFloat("Slow Speed", &rotationSpeedSlow, 0.01f, -10.0f, 10.0f, "%.2f");
        ImGui::DragFloat("Fast Speed", &rotationSpeedFast, 0.01f, -10.0f, 10.0f, "%.2f");
    }

    cmx::Actor::editor();
}

tinyxml2::XMLElement &RotatingActor::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const
{
    tinyxml2::XMLElement &actorElement = cmx::Actor::save(doc, parentElement);

    actorElement.SetAttribute("slowSpeed", rotationSpeedSlow);
    actorElement.SetAttribute("fastSpeed", rotationSpeedFast);

    return actorElement;
}

void RotatingActor::load(tinyxml2::XMLElement *actorElement)
{
    cmx::Actor::load(actorElement);

    rotationSpeedSlow = actorElement->FloatAttribute("slowSpeed");
    rotationSpeedFast = actorElement->FloatAttribute("fastSpeed");
    rotationSpeed = rotationSpeedFast;
}
