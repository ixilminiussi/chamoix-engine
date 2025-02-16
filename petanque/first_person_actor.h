#ifndef FIRST_PERSON_ACTOR
#define FIRST_PERSON_ACTOR

#include "dynamic_body_actor.h"

// cmx
#include <cmx/cmx_camera_component.h>

class FirstPersonActor : public DynamicBodyActor
{
  public:
    using DynamicBodyActor::DynamicBodyActor;

    virtual void onBegin() override;
    virtual void update(float dt) override;

    void onContinuousOverlap(class cmx::PhysicsComponent *ownedComponent,
                             class cmx::PhysicsComponent *overlappingComponent, cmx::Actor *overlappingActor,
                             const cmx::HitInfo &) override;

    void onMovementInput(float dt, glm::vec2 movement);
    void onMouseMovement(float dt, glm::vec2 mousePosition);

    void onShootStart(float, int);
    void onShootRelease(float, int);

    void updateTurnIndicator();

  protected:
    std::shared_ptr<cmx::CameraComponent> _cameraComponent;

    glm::vec3 _oldPosition{0.f};
    float _falling{0.f};
    float _moveSpeed{3.f};
    float _mouseSensitivity{0.02f};

    float _shootTime{0.f};
};

REGISTER_ACTOR(FirstPersonActor)

#endif
