#ifndef CMX_PHYSICS_ACTOR
#define CMX_PHYSICS_ACTOR

#include "cmx_actor.h"
#include "cmx_register.h"

// lib
#include <glm/ext/vector_float3.hpp>

namespace cmx
{

class PhysicsActor : public Actor
{
  public:
    using Actor::Actor;

    void onBegin() override;

    virtual void onBeginOverlap(class PhysicsComponent *ownedComponent, class PhysicsComponent *overlappingComponent,
                                Actor *overlappingActor, const class HitInfo &hitInfo) {};
    virtual void onEndOverlap(class PhysicsComponent *ownedComponent, class PhysicsComponent *overlappingComponent,
                              Actor *overlappingActor) {};
    virtual void onContinuousOverlap(class PhysicsComponent *ownedComponent,
                                     class PhysicsComponent *overlappingComponent, Actor *overlappingActor,
                                     const class HitInfo &hitInfo) {};

  protected:
    std::shared_ptr<class PhysicsComponent> _physicsComponent;

    float _mass{10.f};
};

} // namespace cmx

REGISTER_ACTOR(cmx::PhysicsActor)

#endif
