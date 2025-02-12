#include "enemy_ship_actor.h"

#include "bullet_actor.h"
#include "cmx/cmx_mesh_component.h"
#include "gun_component.h"
#include "ship_camera_component.h"

// cmx
#include <algorithm>
#include <cmx/cmx_camera_component.h>
#include <cmx/cmx_editor.h>
#include <cmx/cmx_game.h>
#include <cmx/cmx_input_action.h>
#include <cmx/cmx_input_manager.h>
#include <cmx/cmx_math.h>
#include <cmx/cmx_physics.h>
#include <cmx/cmx_physics_actor.h>
#include <cmx/cmx_physics_component.h>
#include <cmx/cmx_shapes.h>

// std
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/constants.hpp>
#include <memory>
#include <stdexcept>

void EnemyShipActor::onBegin()
{
    cmx::PhysicsActor::onBegin();

    _transform.scale.x = _transform.scale.y = _transform.scale.z = .6f;

    _gunComponent = std::make_shared<GunComponent>();
    attachComponent(_gunComponent);
    _gunComponent->setGunInfo(GunComponent::enemyDefaultGun);

#ifndef NDEBUG
    if (!cmx::Editor::isActive())
#endif
        cmx::InputManager::setMouseCapture(true);

    _physicsComponent->setPhysicsMode(cmx::PhysicsMode::DYNAMIC);
    _physicsComponent->setMask(0b10000001);
    _physicsComponent->setPosition({0.f, -0.4f, 0.3f});
    _physicsComponent->setScale({3.3f, 3.3f, 3.3f});

    auto meshComponent = std::make_shared<cmx::MeshComponent>();
    attachComponent(meshComponent);
    meshComponent->setModel("enemy-ship");
    meshComponent->setTexture("enemy-ship");
    meshComponent->setScale({50.f, 50.f, 50.f});
    meshComponent->setRotation({glm::pi<float>(), glm::half_pi<float>(), 0.f});

    player = getScene()->getActorByName("Player");
}

void EnemyShipActor::update(float dt)
{
    shoot();
    tiltToPlayer(dt);
    tiltToLocked(dt);
}

void EnemyShipActor::shoot()
{
    try
    {
        _gunComponent->shoot();
    }
    catch (std::out_of_range e)
    {
        spdlog::error("EnemyShipActor: _equippedGun index doesn't match vector");
    }
}

void EnemyShipActor::tiltToPlayer(float dt)
{
    if (!player)
    {
        player = getScene()->getActorByName("Ship");
        return;
    }

    cmx::Transform playerTransform = player->getWorldSpaceTransform();
    cmx::Transform transform = getWorldSpaceTransform();

    glm::vec3 goal = glm::normalize(playerTransform.position - transform.position);

    glm::vec3 axis = glm::cross(glm::vec3{0.f, 0.f, -1.f}, goal);
    float angle = glm::acos(glm::dot(glm::normalize(glm::vec3{0.f, 0.f, -1.f}), goal));
    glm::quat goalRot = glm::angleAxis(angle, glm::normalize(axis));

    _transform.rotation = glm::slerp(_transform.rotation, goalRot, dt * 2.0f);
}

void EnemyShipActor::tiltToLocked(float dt)
{
    glm::vec3 right = _transform.right();
    glm::vec3 forward = _transform.forward();
    static glm::vec3 globalUp{0.f, 1.f, 0.f};

    float angle = glm::acos(glm::dot(right, globalUp));
    glm::vec3 cross = glm::cross(right, globalUp);
    float sign = glm::dot(cross, forward) >= 0.0f ? 1.0f : -1.0f;

    float currentRoll = angle * sign;
    float goalRoll = cmx::snapTo(currentRoll, 0.f, -glm::two_pi<float>(), glm::two_pi<float>(), glm::half_pi<float>(),
                                 -glm::half_pi<float>(), glm::pi<float>(), -glm::pi<float>());

    float lerpedRoll = cmx::lerp(currentRoll, goalRoll, std::clamp(dt * 2.f, 0.f, 1.f));

    glm::quat angledRoll = glm::angleAxis(currentRoll - lerpedRoll, forward);

    _transform.rotation = angledRoll * _transform.rotation;
}

void EnemyShipActor::onBeginOverlap(cmx::PhysicsComponent *ownedComponent, cmx::PhysicsComponent *overlappingComponent,
                                    cmx::Actor *overlappingActor, const cmx::HitInfo &hitInfo)
{
    if (BulletActor *bullet = dynamic_cast<BulletActor *>(overlappingActor))
    {
        _health -= bullet->getDamage();

        if (_health < 0)
        {
            despawn();
        }
        return;
    }

    _transform.position -= (hitInfo.depth + .1f) * hitInfo.normal;
    _physicsComponent->getShape()->reassess();
}
