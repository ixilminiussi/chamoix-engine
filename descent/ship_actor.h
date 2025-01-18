#ifndef SHIP_ACTOR
#define SHIP_ACTOR

#include <cmx/cmx_camera_component.h>
#include <cmx/cmx_physics_actor.h>

class ShipActor : public cmx::PhysicsActor
{
  public:
    using cmx::PhysicsActor::PhysicsActor;

    void onBegin() override;

    void update(float dt) override;

    virtual void onBeginOverlap(class cmx::PhysicsComponent *ownedComponent,
                                class cmx::PhysicsComponent *overlappingComponent,
                                cmx::Actor *overlappingActor) override;
    virtual void onEndOverlap(class cmx::PhysicsComponent *ownedComponent,
                              class cmx::PhysicsComponent *overlappingComponent, cmx::Actor *overlappingActor) override;

    void onMovementInput(float dt, glm::vec2);
    void onViewInput(float dt, glm::vec2);
    void onTiltInput(float dt, glm::vec2);
    void onTiltInputEnd(float dt, int);
    void onLiftInput(float dt, glm::vec2);

  protected:
    void decelerate(float dt, glm::vec3 direction);
    void resetInputs();
    void tiltToLocked(float dt);

    std::shared_ptr<cmx::CameraComponent> _cameraComponent;

    glm::vec3 _velocity{0.f};

    float _movementSpeed{3.f};
    float _movementAcceleration{3.f};
    float _movementDecelerationLerp{.5f};

    bool _movingLeft{false};
    bool _movingRight{false};
    bool _movingForward{false};
    bool _movingBackward{false};
    bool _movingUp{false};
    bool _movingDown{false};

    float _mouseSensitivity{0.1f};

    float _rollSpeed{5.f};
    float _manualRollSpeed{2.f};

    bool _manualTilting{false};
};

#endif
