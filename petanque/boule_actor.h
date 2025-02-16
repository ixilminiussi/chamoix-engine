#ifndef BOULE_ACTOR
#define BOULE_ACTOR

#include "cmx/cmx_mesh_component.h"
#include "rigid_body_actor.h"

class BouleActor : public RigidBodyActor
{
  public:
    enum Team
    {
        BLUE,
        RED,
        COCHONET
    };

    using RigidBodyActor::RigidBodyActor;

    virtual void onBegin() override;
    virtual void update(float dt) override;

    void setTeam(Team);

    Team getTeam() const
    {
        return _team;
    }

    void shoot(glm::vec3 dir, float str);

    void checkClosest();

    static Team whoseTurn();
    static void reset();

  protected:
    static BouleActor *cochonet;
    static BouleActor *closest;

    float _distance;
    Team _team;

    std::shared_ptr<cmx::MeshComponent> _meshComponent;
};

REGISTER_ACTOR(BouleActor)

#endif
