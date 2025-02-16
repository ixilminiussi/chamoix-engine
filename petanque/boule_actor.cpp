#include "boule_actor.h"
#include "cmx/cmx_mesh_component.h"
#include "cmx/cmx_physics_component.h"
#include "cmx/cmx_point_light_component.h"
#include "cmx/cmx_primitives.h"
#include <glm/geometric.hpp>
#include <memory>

BouleActor *BouleActor::cochonet{nullptr};
BouleActor *BouleActor::closest{nullptr};

void BouleActor::onBegin()
{
    RigidBodyActor::onBegin();

    _meshComponent = std::make_shared<cmx::MeshComponent>();

    attachComponent(_meshComponent);
    _meshComponent->setModel("boule-de-petanque");
    _meshComponent->setScale(glm::vec3{2.6f});
    _meshComponent->setTexture("sand");

    _physicsComponent->setShape(PRIMITIVE_SPHERE);
    _physicsComponent->setMask(0b10000000);

    setScale({0.2f, 0.2f, 0.2f});
}

void BouleActor::update(float dt)
{
    RigidBodyActor::update(dt);

    if (_team != COCHONET)
    {
        checkClosest();
    }
}

void BouleActor::setTeam(Team team)
{
    _team = team;

    std::shared_ptr<cmx::PointLightComponent> pointLight = std::make_shared<cmx::PointLightComponent>();

    switch (_team)
    {
    case Team::COCHONET:
        setScale({0.05f, 0.05f, 0.05f});
        _physicsComponent->setMass(.1f);
        _meshComponent->setColor({1.f, 1.f, .4f});
        pointLight->setLightIntensity(0.1f);
        pointLight->setLightColor({1.f, 1.f, .4f});
        attachComponent(pointLight);
        cochonet = this;
        break;
    case Team::BLUE:
        _physicsComponent->setMass(.5f);
        _meshComponent->setColor({0.4f, 0.4f, 1.f});
        break;
    case Team::RED:
        _physicsComponent->setMass(.5f);
        _meshComponent->setColor({1.f, 0.4f, 0.4f});
        break;
    }
}

void BouleActor::shoot(glm::vec3 dir, float str)
{
    _physicsComponent->applyImpulseLinear(dir * str);
}

void BouleActor::checkClosest()
{
    if (cochonet == nullptr)
        return;

    _distance = glm::length(cochonet->getWorldSpaceTransform().position - getWorldSpaceTransform().position);

    if (closest == nullptr)
    {
        closest = this;
    }
    else
    {
        if (closest->_distance > _distance)
        {
            closest = this;
        }
    }
}

BouleActor::Team BouleActor::whoseTurn()
{
    if (cochonet == nullptr)
        return Team::COCHONET;

    if (closest == nullptr)
        return Team::BLUE;

    if (closest->getTeam() == Team::BLUE)
    {
        return Team::RED;
    }
    else
    {
        return Team::BLUE;
    }
}

void BouleActor::reset()
{
    cochonet = nullptr;
    closest = nullptr;
}
