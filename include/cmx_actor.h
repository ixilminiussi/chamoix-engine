#pragma once

#include "cmx_component.h"
#include "cmx_world.h"

// std
#include <cstdlib>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

// lib
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace cmx
{

struct Transform2D
{
    glm::vec2 position = glm::vec2{.0f};
    float rotation = 0;
    glm::vec2 scale = glm::vec2{1.f};

    glm::mat2 getMatrix() const
    {
        float s = sin(rotation);
        float c = cos(rotation);
        glm::mat2 rotationMatrix{{c, s}, {-s, c}};
        glm::mat2 scaleMatrix{{scale.x, 0.f}, {0.f, scale.y}};

        return rotationMatrix * scaleMatrix;
    }
};

class Actor
{
  public:
    template <typename T>
    static std::shared_ptr<T> spawn(World *, const std::string &name, const Transform2D &transform = Transform2D{});

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
    // getters and setters :: end

    // friend functions
    friend void World::addActor(std::shared_ptr<Actor>);
    friend void World::removeActor(Actor *);
    friend std::weak_ptr<Actor> World::getActorByName(std::string &);

    const std::string name;

    Transform2D transform;

  protected:
    Actor(World *, uint32_t id, const std::string &name, const Transform2D &);

    World *world;
    uint32_t id;
    bool isVisible = true;

    std::vector<std::shared_ptr<Component>> components;
};

template <typename T>
std::shared_ptr<T> Actor::spawn(World *world, const std::string &name, const Transform2D &transform)
{
    if constexpr (!std::is_base_of<Actor, T>::value)
    {
        spdlog::critical("'{0}' is not of base type 'Actor', cannot use with 'Actor::spawn'", typeid(T).name());
        std::exit(EXIT_FAILURE);
    }

    static uint32_t currentID = 0;

    Actor *actor = new T{world, currentID++, name, transform};
    auto actorSharedPtr = std::shared_ptr<Actor>(actor);

    world->addActor(actorSharedPtr);
    return std::dynamic_pointer_cast<T>(actorSharedPtr);
}

template <typename T> std::weak_ptr<T> Actor::getComponentByType()
{
    if constexpr (!std::is_base_of<Component, T>::value)
    {
        spdlog::error("'{0}' is not of base type 'Component', 'getComponentByType<{1}>' call is pointless",
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
