#ifndef BULLET_ACTOR
#define BULLET_ACTOR

#include <cmx/cmx_physics_actor.h>

class BulletActor : public cmx::PhysicsActor
{
  public:
    using cmx::PhysicsActor::PhysicsActor;

    void onBegin() override;
    void update(float dt) override;

    void onBeginOverlap(class cmx::PhysicsComponent *ownedComponent, class cmx::PhysicsComponent *overlappingComponent,
                        cmx::Actor *overlappingActor, const cmx::HitInfo &) override;

    void editor() override {}; // should not be edited

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &, tinyxml2::XMLElement *e) override
    {
        return *e;
    }; // cannot be saved
    void load(tinyxml2::XMLElement *) override {}; // cannot be saved

    void setDirection(const glm::vec3 &direction)
    {
        _direction = direction;
    }

  protected:
    float _bulletSpeed = 40.f;
    glm::vec3 _direction{0.f};
    float _scale{.4f};
};

#endif
