#ifndef CMX_ACTOR
#define CMX_ACTOR

// cmx
#include "cmx_component.h"
#include "cmx_scene.h"
#include "cmx_transform.h"

// lib
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <tinyxml2.h>

// std
#include <cstdlib>
#include <memory>
#include <string>
#include <unordered_map>

namespace cmx
{

enum Positioning
{
    Pos_ABSOLUTE,
    Pos_RELATIVE
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
    template <class T> static T *spawn(class Scene *, const char *name, const Transform &transform = Transform{});
    static Actor *duplicate(class Scene *, Actor *actor);

    void despawn();

    Actor() = delete;
    virtual ~Actor();
    Actor(const Actor &) = default;
    Actor &operator=(const Actor &) = delete;

    std::string getType() const;

    virtual void onBegin() {};
    virtual void update(float dt) {};

    virtual tinyxml2::XMLElement &save(tinyxml2::XMLDocument &, tinyxml2::XMLElement *) const;
    virtual void load(tinyxml2::XMLElement *);
    virtual void editor();

    std::shared_ptr<class Component> attachComponent(std::shared_ptr<class Component>, std::string name = "",
                                                     bool force = false);
    void detachComponent(const std::string &name);

    template <typename T> std::weak_ptr<T> getComponentByType();
    std::weak_ptr<class Component> getComponentByName(const std::string &name);
    std::unordered_map<std::string, std::shared_ptr<Component>> &getComponents()
    {
        return _components;
    }

    // getters and setters :: begin
    Scene *getScene()
    {
        return _scene;
    }

    bool isVisible() const
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

    Transform getWorldSpaceTransform() const override;
    const Transform &getLocalSpaceTransform() const override
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

    std::string name;

    Positioning positioning{Positioning::Pos_RELATIVE};

  protected:
    State _state{State::LIVING};

    Actor(Scene *, uint32_t id, const std::string &name, const Transform &);
    std::weak_ptr<Actor> _parent;

    Scene *_scene;
    uint32_t _id;
    bool _isVisible = true;

    std::unordered_map<std::string, std::shared_ptr<Component>> _components{};

    static uint32_t _idProvider;
};

inline uint32_t Actor::_idProvider = 0;

template <typename T> inline T *Actor::spawn(Scene *scene, const char *name, const Transform &transform)
{
    if constexpr (!std::is_base_of<Actor, T>::value)
    {
        throw std::runtime_error(std::string("Actor: '") + typeid(T).name() +
                                 "' is not of base type <Actor>, cannot use with 'Actor::spawn'");
    }

    _idProvider++;

    Actor *actor = new T{scene, _idProvider++, name, transform};

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

#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)

#define UNIQUE_ID CONCAT(__COUNTER__, __LINE__)

#define REGISTER_ACTOR_INTERNAL(Type, ID)                                                                              \
    namespace cmx                                                                                                      \
    {                                                                                                                  \
    class Scene;                                                                                                       \
    }                                                                                                                  \
    struct CONCAT(ActorRegistrar_, ID)                                                                                 \
    {                                                                                                                  \
        CONCAT(ActorRegistrar_, ID)()                                                                                  \
        {                                                                                                              \
            cmx::Register::getInstance().addActor(                                                                     \
                #Type, [](cmx::Scene *scene, const char *name) { return cmx::Actor::spawn<Type>(scene, name); });      \
        }                                                                                                              \
    };                                                                                                                 \
    [[maybe_unused]] inline CONCAT(ActorRegistrar_, ID) CONCAT(actorRegistrar_, ID){};

#define REGISTER_ACTOR(Type) REGISTER_ACTOR_INTERNAL(Type, UNIQUE_ID)

} // namespace cmx

#endif
