#include "cmx_actor.h"

namespace cmx
{

void Actor::spawn(World *world, const std::string &name, const Transform &transform)
{
    static uint32_t currentID = 0;

    Actor *actor = new Actor{world, currentID++, name, transform};

    world->addActor(actor);
}

Actor::Actor(World *world, uint32_t id, const std::string &name, const Transform &transform)
    : world{world}, id{id}, name{name}, transform{transform}
{
}

void Actor::despawn()
{
    getWorld()->removeActor(this);
    // TODO: remove components as well
}

void Actor::move(World *world)
{
    getWorld()->addActor(this);
    getWorld()->removeActor(this);
    // TODO: move components as well
}

void Actor::update(float dt)
{
}

} // namespace cmx
