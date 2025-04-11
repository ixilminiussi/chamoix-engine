#ifndef CMX_SCENE
#define CMX_SCENE

// cmx
#include "cmx_light_environment.h"

// lib
#include <spdlog/spdlog.h>
#include <tinyxml2.h>

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

    class Actor *getActorByName(const std::string &name);
    class Actor *getActorByID(uint32_t id);
    template <typename T> void getAllActorsByType(std::vector<class Actor *> &actorList);
    template <typename T> void getAllComponentsByType(std::vector<std::weak_ptr<class Component>> &componentList);

    std::vector<std::shared_ptr<class Component>> &getAllComponents()
    {
        return _components;
    }

    tinyxml2::XMLElement &save();
    tinyxml2::XMLElement &saveAs(const char *filepath);
    void load(bool skipAssets = false);
    void loadFrom(const std::string &filepath, bool skipAssets = false);
    void unload(bool keepAssets = false);

    void update(float dt);
    void render();

    void addActor(class Actor *);
    void removeActor(class Actor *);
    void addComponent(std::shared_ptr<class Component>);
    void removeComponent(std::shared_ptr<class Component>);

    void setCamera(std::shared_ptr<class Camera> camera);

    const std::weak_ptr<class Camera> getCamera() const
    {
        return _activeCamera;
    }

    class Game *getGame()
    {
        return _game;
    }

    const std::string &getXMLPath() const
    {
        return _xmlPath;
    }

    void setXMLPath(const std::string &xmlPath)
    {
        _xmlPath = xmlPath;
    }

    auto getAssetsManager()
    {
        return _assetsManager.get();
    }

    auto getGraphicsManager()
    {
        return _graphicsManager.get();
    }

    auto getPhysicsManager()
    {
        return _physicsManager.get();
    }

    auto getLightEnvironment()
    {
        return _lightEnvironment.get();
    }

    bool renameActor(Actor *, std::string name);
    std::string name;

  private:
    void updateActors(float dt);
    void updateComponents(float dt);
    void draw();

    std::shared_ptr<class Camera> _activeCamera;
    std::unordered_map<uint32_t, class Actor *> _actors{};
    std::vector<std::shared_ptr<class Component>> _components{};

    std::unique_ptr<class AssetsManager> _assetsManager;
    std::unique_ptr<class GraphicsManager> _graphicsManager;
    std::unique_ptr<class PhysicsManager> _physicsManager;
    std::unique_ptr<class LightEnvironment> _lightEnvironment;

    std::string _xmlPath;
    class Game *_game;
};

template <typename T> inline void Scene::getAllActorsByType(std::vector<Actor *> &actorList)
{
    if constexpr (!std::is_base_of<Actor, T>::value)
    {
        spdlog::error("Scene {0}: '{1}' is not of base type 'Actor', 'getAllActorsByType<{2}>' will return nothing",
                      name, typeid(T).name(), typeid(T).name());
        return;
    }

    for (auto pair : _actors)
    {
        if (T *actor = dynamic_cast<T *>(pair.second))
        {
            actorList.push_back(actor);
        }
    }
}

template <typename T> inline void Scene::getAllComponentsByType(std::vector<std::weak_ptr<Component>> &componentList)
{
    if constexpr (!std::is_base_of<Component, T>::value)
    {
        spdlog::error(
            "Scene {0}: '{1}' is not of base type 'Components', 'getAllComponentsByType<{2}>' will return nothing",
            name, typeid(T).name(), typeid(T).name());
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

#endif
