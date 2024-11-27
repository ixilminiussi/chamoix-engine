#include "cmx_actor.h"

// lib
#include <spdlog/spdlog.h>

// std
#include <memory>
#include <spdlog/common.h>

namespace cmx
{

std::shared_ptr<Actor> Actor::spawn(World *world, const std::string &name, const Transform &transform)
{
    static uint32_t currentID = 0;

    Actor *actor = new Actor{world, currentID++, name, transform};
    std::shared_ptr<Actor> actorSharedPtr = std::shared_ptr<Actor>(actor);

    world->addActor(actorSharedPtr);
    return actorSharedPtr;
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

template <typename T> std::weak_ptr<Component> Actor::getComponentByType()
{
    for (auto component : components)
    {
        if (typeid(T) == typeid(component))
        {
            return component;
        }
    }

    return std::weak_ptr<Component>();
}

void Actor::update(float dt)
{
}

} // namespace cmx
