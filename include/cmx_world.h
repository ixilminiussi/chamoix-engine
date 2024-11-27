#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace cmx
{

class World
{
  public:
    World(const std::string &name) : name(name) {};
    ~World() = default;

    std::weak_ptr<class Actor> getActorByName(std::string &name);
    std::weak_ptr<class Actor> getActorByID(uint32_t id);
    template <typename T> void getAllActorsByType(std::vector<std::weak_ptr<class Actor>> &actorList);
    template <typename T> void getAllComponentsByType(std::vector<std::weak_ptr<class Component>> &componentList);

    std::vector<std::weak_ptr<class Component>> &getAllComponents()
    {
        return components;
    }

    void addActor(std::shared_ptr<class Actor>);
    void removeActor(class Actor *);
    void updateActors(float dt);

    void addComponent(std::shared_ptr<class Component>);
    void updateComponents(float dt);

  private:
    const std::string name;
    std::unordered_map<uint32_t, std::shared_ptr<class Actor>> actors{};
    std::vector<std::weak_ptr<class Component>> components{};
};

} // namespace cmx
