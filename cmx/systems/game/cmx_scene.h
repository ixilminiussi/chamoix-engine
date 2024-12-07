#pragma once

// lib
#include "tinyxml2.h"
#include <spdlog/spdlog.h>

// std
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace cmx
{

class Scene
{
  public:
    Scene(const std::string &name, class Game *game) : name(name), game{game} {};
    ~Scene() = default;

    std::weak_ptr<class Actor> getActorByName(const std::string &name);
    std::weak_ptr<class Actor> getActorByID(uint32_t id);
    template <typename T> void getAllActorsByType(std::vector<std::weak_ptr<class Actor>> &actorList);
    template <typename T> void getAllComponentsByType(std::vector<std::weak_ptr<class Component>> &componentList);

    std::vector<std::weak_ptr<class Component>> &getAllComponents()
    {
        return components;
    }
    std::vector<std::weak_ptr<class Component>> &getRenderQueue()
    {
        return renderQueue;
    }

    void addActor(std::shared_ptr<class Actor>);
    void removeActor(class Actor *);
    void updateActors(float dt);

    void addComponent(std::shared_ptr<class Component>);
    void updateComponents(float dt);
    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &, tinyxml2::XMLElement *);

    void setCamera(std::shared_ptr<class CameraComponent> camera);

    std::weak_ptr<class CameraComponent> getCamera()
    {
        return activeCamera;
    }

    class Game *getGame()
    {
        return game;
    }

    const std::string name;

  private:
    class std::shared_ptr<class CameraComponent> activeCamera;
    std::unordered_map<uint32_t, std::shared_ptr<class Actor>> actors{};
    std::vector<std::weak_ptr<class Component>> components{};
    std::vector<std::weak_ptr<class Component>> renderQueue{};

    class Game *game;
};

template <typename T> inline void Scene::getAllActorsByType(std::vector<std::weak_ptr<Actor>> &actorList)
{
    if constexpr (!std::is_base_of<Actor, T>::value)
    {
        spdlog::error("'{0}' is not of base type 'Actor', 'getAllActorsByType<{1}>' will return nothing",
                      typeid(T).name(), typeid(T).name());
        return;
    }
    for (auto pair : actors)
    {
        if (auto actor = std::dynamic_pointer_cast<T>(pair.second))
        {
            actorList.push_back(actor);
        }
    }
}

template <typename T> inline void Scene::getAllComponentsByType(std::vector<std::weak_ptr<Component>> &componentList)
{
    if constexpr (!std::is_base_of<Component, T>::value)
    {
        spdlog::error("'{0}' is not of base type 'Components', 'getAllComponentsByType<{1}>' will return nothing",
                      typeid(T).name(), typeid(T).name());
        return;
    }
    for (auto component : components)
    {
        if (typeid(T) == typeid(component))
        {
            componentList.push_back(component);
        }
    }
}

} // namespace cmx
