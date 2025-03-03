#ifndef SHIP_ACTOR
#define SHIP_ACTOR

// cmx
#include <cmx_hud_component.h>
#include <cmx_physics_actor.h>

class ShipActor : public cmx::PhysicsActor
{
  public:
    using cmx::PhysicsActor::PhysicsActor;

    void onBegin() override;

    void update(float dt) override;

    void onBeginOverlap(class cmx::PhysicsBody *ownedBody, class cmx::PhysicsBody *overlappingBody,
                        cmx::Actor *overlappingActor, const cmx::HitInfo &) override;
    void onEndOverlap(class cmx::PhysicsBody *ownedBody, class cmx::PhysicsBody *overlappingBody,
                      cmx::Actor *overlappingActor) override;

    void onMovementInput(float dt, glm::vec2);
    void onViewInput(float dt, glm::vec2);
    void onTiltInput(float dt, glm::vec2);
    void onTiltInputEnd(float dt, int);
    void onLiftInput(float dt, glm::vec2);

    void shoot(float dt, int);

  protected:
    void movementDecelerate(float dt, glm::vec3 direction);
    void lookingDecelerate(float dt, glm::vec2 direction);

    void resetInputs();
    void tiltToLocked(float dt);

    std::shared_ptr<class ShipCameraComponent> _cameraComponent;
    std::shared_ptr<cmx::HudComponent> _hudComponent;

    std::vector<std::shared_ptr<class GunComponent>> _gunComponents;
    int _equippedGun = 0;

    float _bounciness{1.5f};

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

    int health{100};
};

REGISTER_ACTOR(ShipActor)

#endif
