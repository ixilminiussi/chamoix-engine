#include "gun_component.h"

#include "bullet_actor.h"
#include "cmx/cmx_transform.h"
#include "ship_camera_component.h"

// cmx
#include <cmx/cmx_actor.h>

GunInfo GunComponent::enemyDefaultGun = GunInfo{.8f, 20, BulletInfo{30.f, .6f, 1, 10, 0b01000000, {1.f, 1.f, 1.f}}};

GunInfo GunComponent::gattlingGun = GunInfo{.15f, 50, BulletInfo{50.f, .4f, 0, 4, 0b10000000, {1.f, 1.f, 1.f}}};

void GunComponent::onAttach()
{
    cmx::Actor *actor = getParent();
    if (actor)
    {
        _cameraComponent = actor->getComponentByType<ShipCameraComponent>();
    }

    reload();
}

void GunComponent::update(float dt)
{
    if (_t > 0)
    {
        _t -= dt;
    }
}

void GunComponent::shoot()
{
    if (_t > 0 || _ammunitions <= 0)
        return;

    cmx::Transform transform;

    if (auto camera = _cameraComponent.lock())
    {
        transform = camera->getAbsoluteTransform();
    }
    else
    {
        transform = getAbsoluteTransform();
    }

    transform.position = transform.position + (transform.up() * -1.0f);

    const std::string name = fmt::format("Bullet_{}", BulletActor::bulletId++);
    BulletActor *actor = cmx::Actor::spawn<BulletActor>(getScene(), name, transform);
    actor->setBulletInfo(_gunInfo.bulletInfo);

    actor->setDirection(transform.forward());

    _t = _gunInfo.timeBetweenShots;
    _ammunitions--;
}

void GunComponent::reload()
{
    _ammunitions = _gunInfo.ammoCapacity;
}
