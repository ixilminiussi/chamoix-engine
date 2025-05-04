#include "ship_actor.h"

#include "bullet_actor.h"
#include "gun_component.h"
#include "ship_camera_component.h"

// cmx
#include <cmx_camera_component.h>
#include <cmx_editor.h>
#include <cmx_game.h>
#include <cmx_input_action.h>
#include <cmx_input_manager.h>
#include <cmx_math.h>
#include <cmx_physics.h>
#include <cmx_physics_actor.h>
#include <cmx_physics_component.h>
#include <cmx_shapes.h>

// std
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/constants.hpp>
#include <memory>
#include <stdexcept>

void ShipActor::onBegin()
{
    cmx::PhysicsActor::onBegin();

    _cameraComponent = std::make_shared<ShipCameraComponent>();
    attachComponent(_cameraComponent);

    _hudComponent = std::make_shared<cmx::HudComponent>();
    attachComponent(_hudComponent);

    cmx::InputManager *inputManager = getScene()->getGame()->getInputManager();
    if (inputManager)
    {
        inputManager->bindAxis("movement", &ShipActor::onMovementInput, this);
        inputManager->bindAxis("look", &ShipActor::onViewInput, this);
        inputManager->bindAxis("look keyboard", &ShipActor::onViewInput, this);
        inputManager->bindAxis("tilt", &ShipActor::onTiltInput, this);
        inputManager->bindAxis("lift", &ShipActor::onLiftInput, this);
        inputManager->bindButton("shoot", &ShipActor::shoot, this);
    }

    _gunComponents.push_back(std::make_shared<GunComponent>());
    attachComponent(_gunComponents[0]);
    _gunComponents[0]->setGunInfo(GunComponent::gattlingGun);

    getScene()->setCamera(_cameraComponent->getCamera());

#ifndef NDEBUG
    if (!cmx::Editor::isActive())
#endif
        cmx::InputManager::setMouseCapture(true);

    _physicsComponent->setPhysicsMode(cmx::PhysicsMode::DYNAMIC);
    _physicsComponent->setMask(0b01000001);
}

void ShipActor::update(float dt)
{
    if (!_manualTilting)
    {
        tiltToLocked(dt);
    }

    if (_manualTilting && std::abs(_tiltingVelocity) > _tiltingSpeed)
    {
        _tiltingVelocity = _tiltingSpeed * (_tiltingVelocity / std::abs(_tiltingVelocity));
    }

    if (_manualTilting && std::abs(_tiltingVelocity) > glm::epsilon<float>())
    {
        glm::quat roll = glm::angleAxis(-_tiltingVelocity * dt, _transform.forward());

        _transform.rotation = roll * _transform.rotation;
    }

    if (glm::length(_movementVelocity) > _movementSpeed)
    {
        _movementVelocity = glm::normalize(_movementVelocity) * _movementSpeed;
    }

    if (glm::length(_movementVelocity) > glm::epsilon<float>())
    {
        if (!_movingForward)
        {
            movementDecelerate(dt, _transform.forward());
        }
        if (!_movingBackward)
        {
            movementDecelerate(dt, -_transform.forward());
        }
        if (!_movingRight)
        {
            movementDecelerate(dt, _transform.right());
        }
        if (!_movingLeft)
        {
            movementDecelerate(dt, -_transform.right());
        }
        if (!_movingUp)
        {
            movementDecelerate(dt, _transform.up());
        }
        if (!_movingDown)
        {
            movementDecelerate(dt, -_transform.up());
        }

        _transform.position += _movementVelocity * dt;
    }

    if (glm::length(_lookingVelocity) > _lookingSpeed)
    {
        _lookingVelocity = glm::normalize(_lookingVelocity) * _lookingSpeed;
    }

    if (glm::length(_lookingVelocity) > glm::epsilon<float>())
    {
        if (!_lookingRight)
        {
            lookingDecelerate(dt, {1.f, 0.f});
        }
        if (!_lookingLeft)
        {
            lookingDecelerate(dt, {-1.f, 0.f});
        }
        if (!_lookingUp)
        {
            lookingDecelerate(dt, {0.f, 1.f});
        }
        if (!_lookingDown)
        {
            lookingDecelerate(dt, {0.f, -1.f});
        }

        glm::quat yaw = glm::angleAxis(-_lookingVelocity.x * dt, _transform.up());
        glm::quat pitch = glm::angleAxis(_lookingVelocity.y * dt, _transform.right());

        _transform.rotation = yaw * _transform.rotation;
        _transform.rotation = pitch * _transform.rotation;
    }

    _cameraComponent->setTilt(_lookingVelocity.x / _lookingSpeed);

    resetInputs();
}

void ShipActor::shoot(float dt, int i)
{
    try
    {
        std::shared_ptr<GunComponent> equippedGun = _gunComponents.at(_equippedGun);
        equippedGun->shoot();
    }
    catch (std::out_of_range e)
    {
        spdlog::error("ShipActor: _equippedGun index doesn't match vector");
    }
}

void ShipActor::movementDecelerate(float dt, glm::vec3 direction)
{
    float rate = glm::dot(_movementVelocity, direction);
    if (rate <= 0)
        return;

    rate = cmx::lerp(0.f, rate, std::clamp(_movementDecelerationLerp * dt, 0.f, 1.f));

    _movementVelocity -= rate * direction;
}

void ShipActor::lookingDecelerate(float dt, glm::vec2 direction)
{
    float rate = glm::dot(_lookingVelocity, direction);
    if (rate <= 0)
        return;

    rate = cmx::lerp(0.f, rate, std::clamp(dt * _lookingDecelerationLerp, 0.f, 1.f));

    _lookingVelocity -= rate * direction;
}

void ShipActor::tiltToLocked(float dt)
{
    glm::vec3 right = _transform.right();
    glm::vec3 forward = _transform.forward();
    static glm::vec3 globalUp{0.f, 1.f, 0.f};

    float angle = glm::acos(glm::dot(right, globalUp));
    glm::vec3 cross = glm::cross(right, globalUp);
    float sign = glm::dot(cross, forward) >= 0.0f ? 1.0f : -1.0f;

    float currentRoll = angle * sign;
    float goalRoll = cmx::snapTo(currentRoll, 0.f, -glm::two_pi<float>(), glm::two_pi<float>(), glm::half_pi<float>(),
                                 -glm::half_pi<float>(), glm::pi<float>(), -glm::pi<float>());

    float lerpedRoll = cmx::lerp(currentRoll, goalRoll, std::clamp(dt * _tiltingLockingLerp, 0.f, 1.f));

    glm::quat angledRoll = glm::angleAxis(currentRoll - lerpedRoll, forward);
    _tiltingVelocity = currentRoll - lerpedRoll;

    _transform.rotation = angledRoll * _transform.rotation;
}

void ShipActor::onBeginOverlap(cmx::PhysicsBody *ownedBody, cmx::PhysicsBody *overlappingBody,
                               cmx::Actor *overlappingActor, const cmx::HitInfo &hitInfo)
{
    _transform.position -= (hitInfo.depth + glm::epsilon<float>()) * hitInfo.normal;
    _movementVelocity = _movementVelocity - 1.5f * glm::dot(_movementVelocity, hitInfo.normal) * hitInfo.normal;
    _physicsComponent->getShape()->reassess();
}

void ShipActor::onEndOverlap(class cmx::PhysicsBody *ownedBody, class cmx::PhysicsBody *overlappingBody,
                             cmx::Actor *overlappingActor)
{
}

void ShipActor::resetInputs()
{
    _movingLeft = false;
    _movingRight = false;
    _movingForward = false;
    _movingBackward = false;
    _movingUp = false;
    _movingDown = false;

    _lookingUp = false;
    _lookingDown = false;
    _lookingLeft = false;
    _lookingRight = false;

    _manualTilting = false;
}

void ShipActor::onMovementInput(float dt, glm::vec2 axis)
{
    if (glm::length(axis) <= glm::epsilon<float>())
        return;

    if (axis.x > 0)
    {
        _movingLeft = true;
    }
    if (axis.x < 0)
    {
        _movingRight = true;
    }
    if (axis.y > 0)
    {
        _movingForward = true;
    }
    if (axis.y < 0)
    {
        _movingBackward = true;
    }

    axis *= _movementAcceleration * dt;

    _movementVelocity += _transform.forward() * axis.y;
    _movementVelocity += _transform.right() * -axis.x;
}

void ShipActor::onViewInput(float dt, glm::vec2 axis)
{
    if (glm::length(axis) <= glm::epsilon<float>())
        return;

    if (axis.x > 0)
    {
        _lookingRight = true;
    }
    if (axis.x < 0)
    {
        _lookingLeft = true;
    }
    if (axis.y < 0)
    {
        _lookingUp = true;
    }
    if (axis.y > 0)
    {
        _lookingDown = true;
    }

    axis *= _lookingAcceleration * dt;

    _lookingVelocity += axis;
}

void ShipActor::onTiltInput(float dt, glm::vec2 axis)
{
    if (glm::length(axis) <= glm::epsilon<float>())
        return;

    _manualTilting = true;

    axis.x *= _tiltingAcceleration * dt;

    _tiltingVelocity += axis.x;
}

void ShipActor::onLiftInput(float dt, glm::vec2 axis)
{
    if (glm::length(axis) <= glm::epsilon<float>())
        return;

    if (axis.y > 0)
    {
        _movingUp = true;
    }
    if (axis.y < 0)
    {
        _movingDown = true;
    }

    axis *= _movementAcceleration * dt;

    _movementVelocity += _transform.up() * axis.y;
}
