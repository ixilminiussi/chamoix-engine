#include "rotating_actor.h"

// cmx
#include "cmx/cmx_game.h"
#include "cmx/cmx_input_manager.h"
#include "cmx/cmx_mesh_component.h"

// lib
#include "imgui.h"
#include <glm/ext/scalar_constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

void RotatingActor::onBegin()
{
    cmx::MeshActor::onBegin();

    auto inputManager = getScene()->getGame()->getInputManager();
    if (inputManager)
    {
        inputManager->bindButton("slowdown toggle", &RotatingActor::slowdownToggle, this);
    }
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
    if (ImGui::CollapsingHeader("Rotation"))
    {
        ImGui::DragFloat("Slow Speed", &rotationSpeedSlow, 0.01f, -10.0f, 10.0f);
        ImGui::DragFloat("Fast Speed", &rotationSpeedFast, 0.01f, -10.0f, 10.0f);
    }

    cmx::MeshActor::renderSettings();
}

tinyxml2::XMLElement &RotatingActor::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement)
{
    tinyxml2::XMLElement &actorElement = cmx::MeshActor::save(doc, parentElement);

    actorElement.SetAttribute("slowSpeed", rotationSpeedSlow);
    actorElement.SetAttribute("fastSpeed", rotationSpeedFast);

    return actorElement;
}

void RotatingActor::load(tinyxml2::XMLElement *actorElement)
{
    cmx::MeshActor::load(actorElement);

    rotationSpeedSlow = actorElement->FloatAttribute("slowSpeed");
    rotationSpeedFast = actorElement->FloatAttribute("fastSpeed");
    rotationSpeed = rotationSpeedFast;
}
