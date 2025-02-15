#include "first_person_actor.h"
#include "cmx/cmx_game.h"
#include "cmx/cmx_input_manager.h"

// cmx
#include <cmx/cmx_camera_component.h>
#include <cmx/cmx_physics_component.h>
#include <cmx/cmx_primitives.h>
#include <cmx/cmx_shapes.h>

void FirstPersonActor::onBegin()
{
    DynamicBodyActor::onBegin();
    _physicsComponent->setShape(PRIMITIVE_SPHERE);

    _cameraComponent = std::make_shared<cmx::CameraComponent>();
    attachComponent(_cameraComponent);

    cmx::InputManager *inputManager = getScene()->getGame()->getInputManager();
    inputManager->bindAxis("movement", &FirstPersonActor::onMovementInput, this);
    inputManager->bindAxis("look", &FirstPersonActor::onMouseMovement, this);

    cmx::InputManager::setMouseCapture(true);

    _oldPosition = _transform.position;
}

void FirstPersonActor::update(float dt)
{
    _falling += 10.f * dt;
    _transform.position.y += _falling * dt;

    glm::vec3 linearVelocity = _transform.position - _oldPosition;
    _physicsComponent->setLinearVelocity(linearVelocity);

    _oldPosition = _transform.position;
}

void FirstPersonActor::onContinuousOverlap(class cmx::PhysicsComponent *ownedComponent,
                                           class cmx::PhysicsComponent *overlappingComponent,
                                           cmx::Actor *overlappingActor, const cmx::HitInfo &hitInfo)
{
    _transform.position -= (hitInfo.depth + glm::epsilon<float>()) * hitInfo.normal;
    _falling = 0.f;
    _physicsComponent->getShape()->reassess();

    glm::vec3 linearVelocity = _physicsComponent->getLinearVelocity();
    linearVelocity.y = 0.f;
    _physicsComponent->setLinearVelocity(linearVelocity);
}

void FirstPersonActor::onMovementInput(float dt, glm::vec2 movement)
{
    if (glm::length(movement) <= glm::epsilon<float>())
        return;

    movement *= _moveSpeed;

    glm::vec3 forward = _cameraComponent->getWorldSpaceForward() * movement.y * dt;
    forward.y = 0;
    _transform.position += forward;
    _transform.position += _cameraComponent->getWorldSpaceRight() * -movement.x * dt;
}

void FirstPersonActor::onMouseMovement(float dt, glm::vec2 mousePosition)
{
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
    glm::quat cameraRotation = _cameraComponent->getLocalSpaceTransform().rotation;
    _cameraComponent->setRotation((yaw * cameraRotation) * pitch);
}
