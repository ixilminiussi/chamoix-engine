#include "bullet_actor.h"

// cmx
#include <cmx/cmx_billboard_component.h>
#include <cmx/cmx_physics.h>
#include <cmx/cmx_physics_component.h>

void BulletActor::onBegin()
{
    cmx::PhysicsActor::onBegin();
    _physicsComponent->setPhysicsMode(cmx::PhysicsMode::DYNAMIC);
    _physicsComponent->setMask(0b10000000);

    _transform.scale = glm::vec3{_scale};

    std::shared_ptr<cmx::BillboardComponent> _billboardComponent = std::make_shared<cmx::BillboardComponent>();
    attachComponent(_billboardComponent);
    _billboardComponent->setHue({1.0f, 0.6f, 0.5f});
}

void BulletActor::update(float dt)
{
    _transform.position += _direction * _bulletSpeed * dt;
}

void BulletActor::onBeginOverlap(cmx::PhysicsComponent *ownedComponent, cmx::PhysicsComponent *overlappingComponent,
                                 cmx::Actor *overlappingActor, const cmx::HitInfo &)
{
    despawn();
}
