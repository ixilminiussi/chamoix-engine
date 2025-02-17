#ifndef ENEMY_SHIP_ACTOR
#define ENEMY_SHIP_ACTOR

// cmx
#include <cmx/cmx_physics_actor.h>

class EnemyShipActor : public cmx::PhysicsActor
{
  public:
    using cmx::PhysicsActor::PhysicsActor;

    void onBegin() override;

    void update(float dt) override;

    void onBeginOverlap(class cmx::PhysicsComponent *ownedComponent, class cmx::PhysicsComponent *overlappingComponent,
                        cmx::Actor *overlappingActor, const cmx::HitInfo &) override;

    void shoot();

  protected:
    void tiltToPlayer(float dt);
    void tiltToLocked(float dt);

    Actor *player;

    std::shared_ptr<class GunComponent> _gunComponent;
    int _equippedGun = 0;

    float _bounciness{1.5f};

    int _health{50};
};

REGISTER_ACTOR(EnemyShipActor)

#endif
