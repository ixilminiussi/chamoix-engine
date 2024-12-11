#include "rotating_actor.h"

// cmx
#include "cmx_game.h"
#include "cmx_mesh_component.h"
#include "imgui.h"

// lib
#include <glm/ext/scalar_constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

void RotatingActor::onBegin()
{
    transform.position = {0.f, 0.f, -5.f};
    transform.scale = {.5f, .5f, .5f};

    auto inputManager = getScene()->getGame()->getInputManager();
    if (inputManager)
    {
        inputManager->bindButton("slowdown toggle", &RotatingActor::slowdownToggle, this);
    }

    auto meshComponent = std::make_shared<cmx::MeshComponent>();
    attachComponent(meshComponent);
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

void RotatingActor::renderSettings(int i)
{
    std::string label;
    ImGui::SeparatorText("Rotation");
    label = fmt::format("Slow Speed##{}", i);
    ImGui::DragFloat(label.c_str(), &rotationSpeedSlow, 0.01f, -10.0f, 10.0f);
    label = fmt::format("Fast Speed##{}", i);
    ImGui::DragFloat(label.c_str(), &rotationSpeedFast, 0.01f, -10.0f, 10.0f);

    Actor::renderSettings(i);
}

tinyxml2::XMLElement &RotatingActor::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement)
{
    tinyxml2::XMLElement &actorElement = Actor::save(doc, parentElement);

    actorElement.SetAttribute("slowSpeed", rotationSpeedSlow);
    actorElement.SetAttribute("fastSpeed", rotationSpeedFast);

    return actorElement;
}

void RotatingActor::load(tinyxml2::XMLElement *actorElement)
{
    Actor::load(actorElement);

    rotationSpeedSlow = actorElement->FloatAttribute("slow speed");
    rotationSpeedFast = actorElement->FloatAttribute("fast speed");
}
