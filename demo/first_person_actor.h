#ifndef FIRST_PERSON_ACTOR
#define FIRST_PERSON_ACTOR

// cmx
#include <cmx_camera_component.h>
#include <cmx_physics_actor.h>
#include <cmx_register.h>

class FirstPersonActor : public cmx::PhysicsActor
{
  public:
    using cmx::PhysicsActor::PhysicsActor;

    virtual void onBegin() override;
    virtual void update(float dt) override;

    void onContinuousOverlap(class cmx::PhysicsBody *ownedComponent, class cmx::PhysicsBody *overlappingComponent,
                             cmx::Actor *overlappingActor, const cmx::HitInfo &) override;

    void onMovementInput(float dt, glm::vec2 movement);
    void onMouseMovement(float dt, glm::vec2 mousePosition);

  protected:
    std::shared_ptr<cmx::CameraComponent> _cameraComponent;

    glm::vec3 _oldPosition{0.f};
    float _falling{0.f};
    float _moveSpeed{3.f};
    float _mouseSensitivity{0.02f};
};

REGISTER_ACTOR(FirstPersonActor)

#endif
