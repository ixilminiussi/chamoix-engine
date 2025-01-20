#ifndef SHIP_ACTOR
#define SHIP_ACTOR

#include "ship_camera_component.h"

// cmx
#include <cmx/cmx_physics_actor.h>

class ShipActor : public cmx::PhysicsActor
{
  public:
    using cmx::PhysicsActor::PhysicsActor;

    void onBegin() override;

    void update(float dt) override;

    virtual void onBeginOverlap(class cmx::PhysicsComponent *ownedComponent,
                                class cmx::PhysicsComponent *overlappingComponent, cmx::Actor *overlappingActor,
                                const cmx::HitInfo &) override;
    virtual void onEndOverlap(class cmx::PhysicsComponent *ownedComponent,
                              class cmx::PhysicsComponent *overlappingComponent, cmx::Actor *overlappingActor) override;

    void onMovementInput(float dt, glm::vec2);
    void onViewInput(float dt, glm::vec2);
    void onTiltInput(float dt, glm::vec2);
    void onTiltInputEnd(float dt, int);
    void onLiftInput(float dt, glm::vec2);

  protected:
    void movementDecelerate(float dt, glm::vec3 direction);
    void lookingDecelerate(float dt, glm::vec2 direction);

    void resetInputs();
    void tiltToLocked(float dt);

    std::shared_ptr<ShipCameraComponent> _cameraComponent;

    glm::vec3 _movementVelocity{0.f};
    float _movementSpeed{20.f};
    float _movementAcceleration{10.f};
    float _movementDecelerationLerp{2.f};

    bool _movingLeft{false};
    bool _movingRight{false};
    bool _movingForward{false};
    bool _movingBackward{false};
    bool _movingUp{false};
    bool _movingDown{false};

    glm::vec2 _lookingVelocity{0.f};
    float _lookingSpeed{2.f};
    float _lookingAcceleration{.5f};
    float _lookingDecelerationLerp{2.f};

    bool _lookingUp{false};
    bool _lookingDown{false};
    bool _lookingRight{false};
    bool _lookingLeft{false};

    float _tiltingVelocity{0.f};
    float _tiltingSpeed{3.f};
    float _tiltingAcceleration{1.f};
    float _tiltingLockingLerp{2.f};

    bool _manualTilting{false};

    float _mouseSensitivity{0.1f};

    float _lastDt;
};

#endif
