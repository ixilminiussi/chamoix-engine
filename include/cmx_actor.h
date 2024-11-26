#pragma once

#include "cmx_world.h"

// std
#include <memory>
#include <string>

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
    static void spawn(World *world, const std::string &name, const Transform &transform = Transform{});

    void despawn();
    void move(World *world);

    Actor() = delete;
    ~Actor() = default;
    Actor &operator=(const Actor &) = delete;
    Actor(const Actor &) = delete;

    void update(float dt);

    // getters and setters :: begin
    World *getWorld()
    {
        return world;
    }
    // getters and setters :: end

    // friend functions
    friend void World::addActor(Actor *actor);
    friend void World::removeActor(Actor *actor);
    friend std::weak_ptr<Actor> World::getActorByName(std::string &name);

  private:
    Actor(World *world, uint32_t id, const std::string &name, const Transform &transform);

    World *world;
    uint32_t id;
    std::string name;
    Transform transform;
};

} // namespace cmx
