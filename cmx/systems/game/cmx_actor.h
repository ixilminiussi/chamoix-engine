#pragma once

#include "cmx_component.h"
#include "cmx_transform.h"
#include "cmx_world.h"

// std
#include <cstdlib>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

namespace cmx
{

enum Positioning
{
    ABSOLUTE,
    RELATIVE
};

class Actor
{
  public:
    template <class T>
    static std::shared_ptr<T> spawn(World *, const std::string &name, const Transform &transform = Transform{});

    void despawn();
    void move(World *);

    Actor() = delete;
    virtual ~Actor() = default;
    Actor &operator=(const Actor &) = delete;
    Actor(const Actor &) = delete;

    virtual void onBegin() {};
    virtual void update(float dt) {};

    void attachComponent(std::shared_ptr<Component>);
    template <typename T> std::weak_ptr<T> getComponentByType();

    // getters and setters :: begin
    World *getWorld()
    {
        return world;
    }

    bool getVisible() const
    {
        return isVisible;
    }

    void setVisible(bool newState)
    {
        isVisible = newState;
    }

    void reParent(std::shared_ptr<Actor> actor)
    {
        parent = actor;
    }

    Transform getAbsoluteTransform();
    // getters and setters :: end

    // friend functions
    friend void World::addActor(std::shared_ptr<Actor>);
    friend void World::removeActor(Actor *);
    friend std::weak_ptr<Actor> World::getActorByName(std::string &);

    const std::string name;

    Transform transform;
    Positioning positioning{Positioning::RELATIVE};

  protected:
    Actor(World *, uint32_t id, const std::string &name, const Transform &);
    std::weak_ptr<Actor> parent;

    World *world;
    uint32_t id;
    bool isVisible = true;

    std::vector<std::shared_ptr<Component>> components;

    static uint32_t currentID;
};

inline uint32_t Actor::currentID = 0;

template <typename T> std::shared_ptr<T> Actor::spawn(World *world, const std::string &name, const Transform &transform)
{
    if constexpr (!std::is_base_of<Actor, T>::value)
    {
        spdlog::critical("'{0}' is not of base type 'Actor', cannot use with 'Actor::spawn'", typeid(T).name());
        std::exit(EXIT_FAILURE);
    }

    currentID++;

    Actor *actor = new T{world, currentID++, name, transform};
    auto actorSharedPtr = std::shared_ptr<Actor>(actor);

    world->addActor(actorSharedPtr);
    return std::dynamic_pointer_cast<T>(actorSharedPtr);
}

template <typename T> std::weak_ptr<T> Actor::getComponentByType()
{
    if constexpr (!std::is_base_of<Component, T>::value)
    {
        spdlog::error(
            "'{0}' is not of base type 'Component', 'getComponentByType<{1}>' will always return invalid pointer",
            typeid(T).name(), typeid(T).name());
        return std::weak_ptr<T>();
    }

    for (const auto &component : components)
    {
        if (auto castedComponent = std::dynamic_pointer_cast<T>(component))
        {
            return castedComponent;
        }
    }

    return std::weak_ptr<T>();
}

} // namespace cmx
