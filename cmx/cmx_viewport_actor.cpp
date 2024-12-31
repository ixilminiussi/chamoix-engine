#include "cmx_viewport_actor.h"

// cmx
#include "cmx_camera_component.h"
#include "cmx_game.h"
#include "cmx_input_manager.h"
#include "cmx_viewport_ui_component.h"
#include "imgui.h"

// lib
#include <glm/common.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/constants.hpp>
#include <vulkan/vulkan_core.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <spdlog/spdlog.h>

// std
#include <memory>

namespace cmx
{

void ViewportActor::onBegin()
{
    transform.position = {0.f, 0.f, 0.f};
    transform.scale = {1.f, 1.f, 1.f};

    _cameraComponent = std::make_shared<CameraComponent>();
    attachComponent(_cameraComponent, "ViewportCamera");
    attachComponent(std::make_shared<ViewportUIComponent>(), "ViewportUI");
}

void ViewportActor::update(float dt)
{
}

void ViewportActor::onMovementInput(float dt, glm::vec2 movement)
{
    if (!_selected)
        return;

    if (glm::length(movement) <= glm::epsilon<float>())
        return;

    movement *= _moveSpeed;

    transform.position += transform.forward() * movement.y * dt;
    transform.position += transform.right() * -movement.x * dt;
}

void ViewportActor::onJumpInput(float dt)
{
    spdlog::info("jumping");
}

void ViewportActor::onMouseMovement(float dt, glm::vec2 mousePosition)
{
    if (!_selected)
        return;

    // Calculate pitch (around X-axis) and yaw (around Y-axis)
    float yawAngle = mousePosition.x * _mouseSensitivity * dt;
    float pitchAngle = -mousePosition.y * _mouseSensitivity * dt;

    // Prevent excessive pitch to avoid gimbal lock (usually between -89 and +89 degrees)
    static float accumulatedPitch = 0.0f;
    pitchAngle = std::min(glm::half_pi<float>() - 0.01f - accumulatedPitch, pitchAngle);
    pitchAngle = std::max(-glm::half_pi<float>() + 0.01f - accumulatedPitch, pitchAngle);
    accumulatedPitch += pitchAngle;
    accumulatedPitch = glm::clamp(accumulatedPitch, -glm::half_pi<float>() + 0.01f, glm::half_pi<float>() - 0.01f);

    // Create quaternions for yaw and pitch
    glm::quat yaw = glm::angleAxis(yawAngle, glm::vec3(0.0f, 1.0f, 0.0f));      // Rotate around the Y-axis
    glm::quat pitch = glm::angleAxis(-pitchAngle, glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate around the X-axis

    // Combine the new rotations: yaw first, then pitch relative to yaw
    transform.rotation = yaw * transform.rotation;   // Apply yaw
    transform.rotation = transform.rotation * pitch; // Apply pitch in local space

    // Ensure the quaternion remains normalized
    transform.rotation = glm::normalize(transform.rotation);
}

void ViewportActor::select(float dt, int val)
{
    if (val == 1)
    {
        _selected = true;
        getScene()->getGame()->getInputManager()->setMouseCapture(true);
    }
    else
    {
        _selected = false;
        getScene()->getGame()->getInputManager()->setMouseCapture(false);
    }
}

tinyxml2::XMLElement &ViewportActor::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement)
{
    tinyxml2::XMLElement &actorElement = Actor::save(doc, parentElement);

    return actorElement;
}

void ViewportActor::load(tinyxml2::XMLElement *actorElement)
{
    Actor::load(actorElement);
}

void ViewportActor::editor()
{
    ImGui::SliderFloat("movement speed", &_moveSpeed, 0.0f, 100.0f);
    ImGui::SliderFloat("mouse sensitivity", &_mouseSensitivity, 0.0f, 10.0f);
}

} // namespace cmx
