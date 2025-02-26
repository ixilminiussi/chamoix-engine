#ifndef BULLET_ACTOR
#define BULLET_ACTOR

#include <cmx/cmx_billboard_component.h>
#include <cmx/cmx_component.h>
#include <cmx/cmx_physics_actor.h>

struct BulletInfo
{
    float speed{40.f};
    float scale{.4f};
    int bounceCount{0};
    int damage{10};
    uint8_t mask{0b10000000};
    glm::vec3 color{1.f, .4f, .35f};
};

class BulletActor : public cmx::PhysicsActor
{
  public:
    using cmx::PhysicsActor::PhysicsActor;

    void onBegin() override;
    void update(float dt) override;

    void onBeginOverlap(class cmx::PhysicsComponent *ownedComponent, class cmx::PhysicsComponent *overlappingComponent,
                        cmx::Actor *overlappingActor, const cmx::HitInfo &) override;

    void editor() override {}; // should not be edited

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &, tinyxml2::XMLElement *e) const override
    {
        return *e;
    }; // cannot be saved
    void load(tinyxml2::XMLElement *) override {}; // cannot be saved

    void setBulletInfo(const BulletInfo &);
    void setDirection(const glm::vec3 &direction)
    {
        _direction = direction;
    }

    int getDamage() const
    {
        return _damage;
    }

    static unsigned int bulletId;

  protected:
    float _bulletSpeed = 40.f;
    glm::vec3 _direction{0.f};
    float _scale{.4f};

    int _bounceCount{0};

    int _damage{10};

    std::shared_ptr<cmx::BillboardComponent> _billboardComponent;
};

REGISTER_ACTOR(BulletActor)

#endif
