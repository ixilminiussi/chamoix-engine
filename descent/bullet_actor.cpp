#include "bullet_actor.h"
#include "cmx/cmx_shapes.h"

// cmx
#include <cmx/cmx_billboard_component.h>
#include <cmx/cmx_physics.h>
#include <cmx/cmx_physics_component.h>

unsigned int BulletActor::bulletId{0};

void BulletActor::onBegin()
{
    cmx::PhysicsActor::onBegin();
    _physicsComponent->setPhysicsMode(cmx::PhysicsMode::DYNAMIC);
    _physicsComponent->setMask(0b10000000);

    _transform.scale = glm::vec3{_scale};

    _billboardComponent = std::make_shared<cmx::BillboardComponent>();
    attachComponent(_billboardComponent);
    _billboardComponent->setTexture("fire_ball");
}

void BulletActor::update(float dt)
{
    _transform.position += _direction * _bulletSpeed * dt;
}

void BulletActor::setBulletInfo(const BulletInfo &info)
{
    _bulletSpeed = info.speed;
    _scale = info.speed;
    _bounceCount = info.bounceCount;
    _damage = info.damage;

    _billboardComponent->setHue(info.color);

    _physicsComponent->setMask(info.mask);
}

void BulletActor::onBeginOverlap(cmx::PhysicsComponent *ownedComponent, cmx::PhysicsComponent *overlappingComponent,
                                 cmx::Actor *overlappingActor, const cmx::HitInfo &hitInfo)
{
    if (_bounceCount <= 0)
    {
        despawn();
    }
    else
    {
        _bounceCount--;

        _transform.position -= (hitInfo.depth + glm::epsilon<float>()) * hitInfo.normal;
        _direction = _direction - 2.f * glm::dot(_direction, hitInfo.normal) * hitInfo.normal;
    }
}
