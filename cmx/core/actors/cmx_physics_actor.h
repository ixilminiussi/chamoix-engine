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

    virtual void onBeginOverlap(class PhysicsBody *ownedComponent, class PhysicsBody *overlappingComponent,
                                Actor *overlappingActor, const struct HitInfo &hitInfo) {};
    virtual void onEndOverlap(class PhysicsBody *ownedComponent, class PhysicsBody *overlappingComponent,
                              Actor *overlappingActor) {};
    virtual void onContinuousOverlap(class PhysicsBody *ownedComponent, class PhysicsBody *overlappingComponent,
                                     Actor *overlappingActor, const struct HitInfo &hitInfo) {};

  protected:
    std::shared_ptr<class PhysicsComponent> _physicsComponent;

    float _mass{10.f};
};

} // namespace cmx

REGISTER_ACTOR(cmx::PhysicsActor)

#endif
