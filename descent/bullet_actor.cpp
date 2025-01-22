#include "bullet_actor.h"
#include "cmx/cmx_billboard_component.h"

void BulletActor::onBegin()
{
    std::shared_ptr<cmx::BillboardComponent> _billboardComponent = std::make_shared<cmx::BillboardComponent>();
    attachComponent(_billboardComponent);
    _billboardComponent->setHue({1.0f, 0.6f, 0.5f});
}

void BulletActor::update(float dt)
{
    _transform.position += _direction * _bulletSpeed * dt;
}

void BulletActor::onBeginOverlap(class cmx::PhysicsComponent *ownedComponent,
                                 class cmx::PhysicsComponent *overlappingComponent, cmx::Actor *overlappingActor,
                                 const cmx::HitInfo &)
{
}
