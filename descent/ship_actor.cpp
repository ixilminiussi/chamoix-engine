#include "ship_actor.h"

// cmx
#include <cmx/cmx_camera_component.h>
#include <cmx/cmx_game.h>
#include <cmx/cmx_input_manager.h>
#include <cmx/cmx_math.h>

// std
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/constants.hpp>
#include <memory>

void ShipActor::onBegin()
{
    _cameraComponent = std::make_shared<cmx::CameraComponent>();
    attachComponent(_cameraComponent);

    if (auto inputManager = getScene()->getGame()->getInputManager())
    {
        inputManager->bindAxis("Movement", &ShipActor::onMovementInput, this);
        inputManager->bindAxis("View", &ShipActor::onViewInput, this);
        inputManager->bindAxis("View Keyboard", &ShipActor::onViewInput, this);
        inputManager->bindAxis("Tilt", &ShipActor::onTiltInput, this);
        inputManager->bindAxis("Lift", &ShipActor::onLiftInput, this);
        inputManager->bindButton("Tilt end", &ShipActor::onTiltInputEnd, this);
    }

    getScene()->setCamera(_cameraComponent->getCamera());
}

void ShipActor::update(float dt)
{
    if (!_manualTilting)
    {
        tiltToLocked(dt);
    }
}

void ShipActor::tiltToLocked(float dt)
{
    glm::vec3 right = transform.right();
    glm::vec3 forward = transform.forward();
    static glm::vec3 globalUp{0.f, 1.f, 0.f};

    float angle = glm::acos(glm::dot(right, globalUp));
    glm::vec3 cross = glm::cross(right, globalUp);
    float sign = glm::dot(cross, forward) >= 0.0f ? 1.0f : -1.0f;

    float currentRoll = angle * sign;
    float goalRoll = cmx::snapTo(currentRoll, 0.f, -glm::two_pi<float>(), glm::two_pi<float>(), glm::half_pi<float>(),
                                 -glm::half_pi<float>(), glm::pi<float>(), -glm::pi<float>());

    float lerpedRoll = cmx::lerp(currentRoll, goalRoll, std::clamp(dt * _rollSpeed, 0.f, 1.f));
    glm::quat angledRoll = glm::angleAxis(currentRoll - lerpedRoll, transform.forward());

    transform.rotation = angledRoll * transform.rotation;
}

void ShipActor::onBeginOverlap(class cmx::PhysicsComponent *ownedComponent,
                               class cmx::PhysicsComponent *overlappingComponent, cmx::Actor *overlappingActor)
{
}

void ShipActor::onEndOverlap(class cmx::PhysicsComponent *ownedComponent,
                             class cmx::PhysicsComponent *overlappingComponent, cmx::Actor *overlappingActor)
{
}

void ShipActor::onMovementInput(float dt, glm::vec2 axis)
{
    if (glm::length(axis) <= glm::epsilon<float>())
        return;

    axis *= _movementSpeed;

    transform.position += transform.forward() * axis.y * dt;
    transform.position += transform.right() * -axis.x * dt;
}

void ShipActor::onViewInput(float dt, glm::vec2 axis)
{
    if (glm::length(axis) <= glm::epsilon<float>())
        return;

    float yawAngle = -axis.x * _mouseSensitivity * dt;
    float pitchAngle = axis.y * _mouseSensitivity * dt;

    glm::quat yaw = glm::angleAxis(yawAngle, transform.up());
    glm::quat pitch = glm::angleAxis(pitchAngle, transform.right());

    transform.rotation = yaw * transform.rotation;
    transform.rotation = pitch * transform.rotation;

    transform.rotation = glm::normalize(transform.rotation);
}

void ShipActor::onTiltInput(float dt, glm::vec2 axis)
{
    if (glm::length(axis) <= glm::epsilon<float>())
        return;

    float rollAngle = axis.x * _manualRollSpeed * dt;

    glm::quat roll = glm::angleAxis(rollAngle, transform.forward());

    transform.rotation = roll * transform.rotation;

    _manualTilting = true;
}

void ShipActor::onTiltInputEnd(float dt, int val)
{
    _manualTilting = false;
}

void ShipActor::onLiftInput(float dt, glm::vec2 axis)
{
    if (glm::length(axis) <= glm::epsilon<float>())
        return;

    axis *= _liftSpeed;

    transform.position += transform.up() * axis.y * dt;
}
