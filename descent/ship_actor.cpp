#include "ship_actor.h"

// cmx
#include <cmx/cmx_camera_component.h>
#include <cmx/cmx_game.h>
#include <cmx/cmx_input_manager.h>

// std
#include <glm/ext/scalar_constants.hpp>
#include <memory>

void ShipActor::onBegin()
{
    _cameraComponent = std::make_shared<cmx::CameraComponent>();
    attachComponent(_cameraComponent);

    if (auto inputManager = getScene()->getGame()->getInputManager())
    {
        inputManager->bindAxis("Movement", &ShipActor::onMovementInput, this);
        inputManager->bindAxis("View", &ShipActor::onViewInput, this);
        inputManager->bindAxis("Tilt", &ShipActor::onTiltInput, this);
        inputManager->bindAxis("Lift", &ShipActor::onLiftInput, this);
        inputManager->bindButton("Tilt end", &ShipActor::onTiltInputEnd, this);
    }
}

void ShipActor::update(float dt)
{
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
    if (!_selected)
        return;

    if (glm::length(axis) <= glm::epsilon<float>())
        return;

    axis *= _movementSpeed;

    transform.position += transform.forward() * axis.y * dt;
    transform.position += transform.right() * -axis.x * dt;
}

void ShipActor::onViewInput(float dt, glm::vec2 axis)
{
    if (!_selected)
        return;

    if (glm::length(axis) <= glm::epsilon<float>())
        return;

    float yawAngle = axis.x * _mouseSensitivity * dt;
    float pitchAngle = -axis.y * _mouseSensitivity * dt;

    glm::quat yaw = glm::angleAxis(yawAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat pitch = glm::angleAxis(-pitchAngle, glm::vec3(1.0f, 0.0f, 0.0f));

    transform.rotation = yaw * transform.rotation;
    transform.rotation = pitch * transform.rotation;

    transform.rotation = glm::normalize(transform.rotation);
}

void ShipActor::onTiltInput(float dt, glm::vec2 axis)
{
    if (!_selected)
        return;

    if (glm::length(axis) <= glm::epsilon<float>())
        return;

    float rollAngle = axis.x * _mouseSensitivity * dt;

    glm::quat roll = glm::angleAxis(rollAngle, glm::vec3(0.0f, 0.0f, 1.0f));

    transform.rotation = roll * transform.rotation;

    transform.rotation = glm::normalize(transform.rotation);

    _tilting = true;
}

void ShipActor::onTiltInputEnd(float dt, int val)
{
    _tilting = false;
}

void ShipActor::onLiftInput(float dt, glm::vec2 axis)
{
    if (!_selected)
        return;

    if (glm::length(axis) <= glm::epsilon<float>())
        return;

    axis *= _liftSpeed;

    transform.position += transform.up() * axis.y * dt;
}

void ShipActor::select()
{
    _selected = true;
    getScene()->setCamera(_cameraComponent->getCamera());
}
