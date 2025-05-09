#include "cmx_viewport_actor.h"

// cmx
#include "cmx_camera.h"
#include "cmx_editor.h"
#include "cmx_game.h"
#include "cmx_input_manager.h"

// lib
#include "imgui.h"
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

ViewportActor::ViewportActor()
{
    _camera = std::make_shared<Camera>();
}

void ViewportActor::update(float dt)
{
    _camera->setViewDirection(_transform.position, _transform.forward(), _transform.up());
}

void ViewportActor::onMovementInput(float dt, glm::vec2 movement)
{
    if (!_selected || !Editor::isActive())
        return;

    if (glm::length(movement) <= glm::epsilon<float>())
        return;

    movement *= _moveSpeed;

    _transform.position += _transform.forward() * movement.y * dt;
    _transform.position += _transform.right() * -movement.x * dt;
}

void ViewportActor::onMouseMovement(float dt, glm::vec2 mousePosition)
{
    if (!_selected || !Editor::isActive())
        return;

    dt = std::min(.05f, dt);

    // Calculate pitch
    float yawAngle = mousePosition.x * _mouseSensitivity * dt;
    float pitchAngle = -mousePosition.y * _mouseSensitivity * dt;

    // Prevent excessive pitch to avoid gimbal lock
    static float accumulatedPitch = 0.0f;
    pitchAngle = std::min(glm::half_pi<float>() - 0.01f - accumulatedPitch, pitchAngle);
    pitchAngle = std::max(-glm::half_pi<float>() + 0.01f - accumulatedPitch, pitchAngle);
    accumulatedPitch += pitchAngle;
    accumulatedPitch = glm::clamp(accumulatedPitch, -glm::half_pi<float>() + 0.01f, glm::half_pi<float>() - 0.01f);

    // Create quaternions for yaw and pitch
    glm::quat yaw = glm::angleAxis(yawAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat pitch = glm::angleAxis(-pitchAngle, glm::vec3(1.0f, 0.0f, 0.0f));

    // Combine the new rotations
    _transform.rotation = yaw * _transform.rotation;
    _transform.rotation = _transform.rotation * pitch;

    _transform.rotation = glm::normalize(_transform.rotation);
}

void ViewportActor::select(float dt, int val)
{
    if (_locked || (!Editor::getInstance()->isViewportActorHovered() && !_selected))
    {
        return;
        _selected = false;
    }

    if (val == 1)
    {
        _selected = true;
        InputManager::setMouseCapture(true, true);
    }
    else
    {
        _selected = false;
        InputManager::setMouseCapture(false, true);
    }
}

void ViewportActor::updateMoveSpeed(float dt, glm::vec2 vec)
{
    _moveSpeed *= 1.f + vec.y * 0.1f;
}

void ViewportActor::editor()
{
    ImGui::SliderFloat("movement speed", &_moveSpeed, 0.0f, 100.0f);
    ImGui::SliderFloat("mouse sensitivity", &_mouseSensitivity, 0.0f, 2.0f);
}

} // namespace cmx
