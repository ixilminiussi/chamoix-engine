#include "cmx_world.h"

#include "cmx_actor.h"

// std
#include <memory>
#include <stdexcept>

// lib
#include <spdlog/spdlog.h>

namespace cmx
{

std::weak_ptr<Actor> World::getActorByName(std::string &name)
{
    for (auto pair : actors)
    {
        if (pair.second->name == name)
        {
            return pair.second;
        }
    }

    return std::weak_ptr<Actor>();
}

std::weak_ptr<Actor> World::getActorByID(uint32_t id)
{
    std::weak_ptr<Actor> actor;
    try
    {
        actor = actors.at(id);
    }
    catch (const std::out_of_range &e)
    {
        spdlog::warn("World '{0}': Attempt to get actor from invalid id: '{1}''", name, id);
    }

    return actor;
}

void World::addActor(Actor *actor)
{
#if DEBUG
    // expensive operation so we only use it in debug mode
    if (getActorByName(actor->name).isValid())
    {
        spdlog::warn("World '{0}': An actor with name: '{0}' already exists", name, actor->name);
    }
#endif

    actors[actor->id] = std::shared_ptr<Actor>(actor);
    spdlog::info("World '{0}': Added new actor: '{1}'", name, actor->name);
}

void World::removeActor(Actor *actor)
{
    try
    {
        actors.at(actor->id) = nullptr;
    }
    catch (const std::out_of_range &e)
    {
        spdlog::warn("World '{0}': Attempt to remove non-managed actor: '{1}''", name, actor->name);
    }

    actors.erase(actor->id);

    spdlog::info("World '{0}': Removed actor: '{1}'", name, actor->name);
}

void World::updateActors(float dt)
{
    for (auto pair : actors)
    {
        pair.second->update(dt);
    }
}

} // namespace cmx
