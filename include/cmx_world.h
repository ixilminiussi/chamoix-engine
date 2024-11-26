#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

namespace cmx
{

class World
{
  public:
    World(const std::string &name) : name(name)
    {
    }
    ~World() = default;

    std::weak_ptr<class Actor> getActorByName(std::string &name);
    std::weak_ptr<class Actor> getActorByID(uint32_t id);

    void addActor(class Actor *actor);
    void removeActor(class Actor *actor);

    void updateActors(float dt);

  private:
    const std::string name;
    std::unordered_map<uint32_t, std::shared_ptr<class Actor>> actors{};
};

} // namespace cmx
