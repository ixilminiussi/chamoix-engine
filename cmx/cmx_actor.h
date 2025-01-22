#ifndef CMX_ACTOR
#define CMX_ACTOR

#include "cmx_scene.h"
#include "cmx_transform.h"

// lib
#include <tinyxml2.h>

// std
#include <cstdlib>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>
#include <unordered_map>

namespace cmx
{

enum Positioning
{
    ABSOLUTE,
    RELATIVE
};

enum State
{
    LIVING,
    DEAD,
    PREFER_NOT_TO_SAY
};

class Actor : public std::enable_shared_from_this<Actor>, public Transformable
{
  public:
    template <class T>
    static T *spawn(class Scene *, const std::string &name, const Transform &transform = Transform{});

    void despawn();

    Actor() = delete;
    virtual ~Actor();
    Actor &operator=(const Actor &) = delete;
    Actor(const Actor &) = delete;

    std::string getType();

    virtual void onBegin() {};
    virtual void update(float dt) {};

    virtual tinyxml2::XMLElement &save(tinyxml2::XMLDocument &, tinyxml2::XMLElement *);
    virtual void load(tinyxml2::XMLElement *);
    virtual void editor();

    std::shared_ptr<class Component> attachComponent(std::shared_ptr<class Component>, std::string name = "");
    void detachComponent(const std::string &name);
    template <typename T> std::weak_ptr<T> getComponentByType();
    std::weak_ptr<class Component> getComponentByName(const std::string &name);

    // getters and setters :: begin
    Scene *getScene()
    {
        return _scene;
    }

    bool getVisible() const
    {
        return _isVisible;
    }

    void setVisible(bool newState)
    {
        _isVisible = newState;
    }

    void reparent(std::shared_ptr<Actor> actor)
    {
        _parent = actor;
    }

    uint32_t getID()
    {
        return _id;
    }

    Transform getAbsoluteTransform() const override;
    const Transform &getRelativeTransform() const override
    {
        return _transform;
    }

    bool markedForDeletion()
    {
        return _state == State::DEAD;
    }
    // getters and setters :: end

    // friend functions
    friend void Scene::addActor(Actor *);
    friend void Scene::removeActor(Actor *);
    friend Actor *Scene::getActorByName(const std::string &);

    const std::string name;

    Positioning positioning{Positioning::RELATIVE};

  protected:
    State _state{State::LIVING};

    Actor(Scene *, uint32_t id, const std::string &name, const Transform &);
    std::weak_ptr<Actor> _parent;

    Scene *_scene;
    uint32_t _id;
    bool _isVisible = true;

    std::unordered_map<std::string, std::shared_ptr<Component>> _components{};

    static uint32_t currentID;
};

inline uint32_t Actor::currentID = 0;

template <typename T> inline T *Actor::spawn(Scene *scene, const std::string &name, const Transform &transform)
{
    if constexpr (!std::is_base_of<Actor, T>::value)
    {
        spdlog::critical("'{0}' is not of base type 'Actor', cannot use with 'Actor::spawn'", typeid(T).name());
        std::exit(EXIT_FAILURE);
    }

    currentID++;

    Actor *actor = new T{scene, currentID++, name, transform};

    scene->addActor(actor);

    return (T *)actor;
}

template <typename T> inline std::weak_ptr<T> Actor::getComponentByType()
{
    if constexpr (!std::is_base_of<Component, T>::value)
    {
        spdlog::error(
            "'{0}' is not of base type 'Component', 'getComponentByType<{1}>' will always return invalid pointer",
            typeid(T).name(), typeid(T).name());
        return std::weak_ptr<T>();
    }

    for (const auto &component : _components)
    {
        if (auto castedComponent = std::dynamic_pointer_cast<T>(component.second))
        {
            return castedComponent;
        }
    }

    return std::weak_ptr<T>();
}

} // namespace cmx

#endif
