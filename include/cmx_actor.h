#pragma once

#include "cmx_component.h"
#include "cmx_world.h"

// std
#include <memory>
#include <string>
#include <vector>

// lib
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace cmx
{

struct Transform
{
    glm::vec3 position = glm::vec3{.0f};
    glm::vec3 rotation = glm::vec3{1.f};
    glm::vec3 scale = glm::vec3{1.f};
};

class Actor
{
  public:
    static std::shared_ptr<Actor> spawn(World *, const std::string &name, const Transform &transform = Transform{});

    void despawn();
    void move(World *);

    Actor() = delete;
    ~Actor() = default;
    Actor &operator=(const Actor &) = delete;
    Actor(const Actor &) = delete;

    void update(float dt);

    void attachComponent(std::shared_ptr<Component>);
    template <typename T> std::weak_ptr<Component> getComponentByType();

    // getters and setters :: begin
    World *getWorld()
    {
        return world;
    }

    bool getVisible()
    {
        return isVisible;
    }

    void setVisible(bool newState)
    {
        isVisible = newState;
    }

    const Transform &getTransform()
    {
        return transform;
    }

    void setPosition(const glm::vec3 &position)
    {
        transform.position = position;
    }

    void setScale(const glm::vec3 &scale)
    {
        transform.scale = scale;
    }
    // getters and setters :: end

    // friend functions
    friend void World::addActor(std::shared_ptr<Actor>);
    friend void World::removeActor(Actor *);
    friend std::weak_ptr<Actor> World::getActorByName(std::string &);

    const std::string name;

  protected:
    Actor(World *, uint32_t id, const std::string &name, const Transform &);

    World *world;
    uint32_t id;
    Transform transform;
    bool isVisible = true;

    std::vector<std::shared_ptr<Component>> components;
};

} // namespace cmx
