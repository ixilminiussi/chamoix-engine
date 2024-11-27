#include "cmx_actor.h"

// lib
#include <spdlog/spdlog.h>

// std
#include <memory>
#include <spdlog/common.h>

namespace cmx
{

Actor::Actor(World *world, uint32_t id, const std::string &name, const Transform2D &transform)
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
    getWorld()->addActor(std::shared_ptr<Actor>(this));
    getWorld()->removeActor(this);
    // TODO: move components as well
}

void Actor::attachComponent(std::shared_ptr<Component> component)
{
    component->setParent(this);

    components.push_back(component);
    getWorld()->addComponent(component);
}

} // namespace cmx
