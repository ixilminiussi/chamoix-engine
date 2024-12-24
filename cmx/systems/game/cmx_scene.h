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
    Scene(const std::string &xmlPath, class Game *, const std::string &name);
    ~Scene();

    std::weak_ptr<class Actor> getActorByName(const std::string &name);
    std::weak_ptr<class Actor> getActorByID(uint32_t id);
    template <typename T> void getAllActorsByType(std::vector<std::weak_ptr<class Actor>> &actorList);
    template <typename T> void getAllComponentsByType(std::vector<std::weak_ptr<class Component>> &componentList);

    std::vector<std::weak_ptr<class Component>> &getAllComponents()
    {
        return _components;
    }

    tinyxml2::XMLElement &save();
    tinyxml2::XMLElement &saveAs(const char *filepath);
    void load();
    void unload();

    void update(float dt);

    std::shared_ptr<Actor> addActor(std::shared_ptr<class Actor>);
    void removeActor(class Actor *);
    void addComponent(std::shared_ptr<class Component>);

    void setCamera(std::shared_ptr<class CameraComponent> camera);

    std::weak_ptr<class CameraComponent> getCamera()
    {
        return _activeCamera;
    }

    class Game *getGame()
    {
        return _game;
    }

    const std::string _xmlPath;
    const std::string name;

    std::shared_ptr<class AssetsManager> _assetsManager;
    std::unique_ptr<class GraphicsManager> _graphicsManager;

  private:
    void updateActors(float dt);
    void updateComponents(float dt);
    void draw();

    std::shared_ptr<class CameraComponent> _activeCamera;
    std::unordered_map<uint32_t, std::shared_ptr<class Actor>> _actors{};
    std::vector<std::weak_ptr<class Component>> _components{};

    class Game *_game;
};

template <typename T> inline void Scene::getAllActorsByType(std::vector<std::weak_ptr<Actor>> &actorList)
{
    if constexpr (!std::is_base_of<Actor, T>::value)
    {
        spdlog::error("'{0}' is not of base type 'Actor', 'getAllActorsByType<{1}>' will return nothing",
                      typeid(T).name(), typeid(T).name());
        return;
    }

    for (auto pair : _actors)
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
    for (auto component : _components)
    {
        if (typeid(T) == typeid(component))
        {
            componentList.push_back(component);
        }
    }
}

} // namespace cmx
