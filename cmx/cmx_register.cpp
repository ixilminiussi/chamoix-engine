#include "cmx_register.h"

// cmx
#include "cmx_actor.h"
#include "cmx_camera_component.h"
#include "cmx_component.h"
#include "cmx_mesh_actor.h"
#include "cmx_mesh_component.h"
#include "cmx_physics_actor.h"
#include "cmx_physics_component.h"
#include "cmx_point_light_actor.h"
#include "cmx_point_light_component.h"
#include <memory>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace cmx
{

Register *Register::_instance{};

Register::Register()
{
    actorRegister["cmx::Actor"] = [](class Scene *scene, const std::string &name) {
        return Actor::spawn<Actor>(scene, name);
    };
    actorRegister["cmx::MeshActor"] = [](class Scene *scene, const std::string &name) {
        return Actor::spawn<MeshActor>(scene, name);
    };
    actorRegister["cmx::PointLightActor"] = [](class Scene *scene, const std::string &name) {
        return Actor::spawn<PointLightActor>(scene, name);
    };
    actorRegister["cmx::PhysicsActor"] = [](class Scene *scene, const std::string &name) {
        return Actor::spawn<PhysicsActor>(scene, name);
    };

    componentRegister["cmx::Component"] = []() { return std::make_shared<Component>(); };
    componentRegister["cmx::MeshComponent"] = []() { return std::make_shared<MeshComponent>(); };
    componentRegister["cmx::PointLightComponent"] = []() { return std::make_shared<PointLightComponent>(); };
    componentRegister["cmx::CameraComponent"] = []() { return std::make_shared<CameraComponent>(); };
    componentRegister["cmx::PhysicsComponent"] = []() { return std::make_shared<PhysicsComponent>(); };
}

Register::~Register()
{
}

Register *Register::getInstance()
{
    if (_instance == nullptr)
    {
        _instance = new Register();
    }

    return _instance;
}

void Register::addActor(std::string name,
                        std::function<std::shared_ptr<Actor>(class Scene *, const std::string &)> builder)
{
    try
    {
        actorRegister.at(name);
        spdlog::warn("Register: duplicate actor '{0}' in register", name);
    }
    catch (std::out_of_range e)
    {
        actorRegister[name] = builder;
    }
}

void Register::addComponent(std::string name, std::function<std::shared_ptr<class Component>()> builder)
{
    try
    {
        componentRegister.at(name);
        spdlog::warn("Register: duplicate component '{0}' in register", name);
    }
    catch (std::out_of_range e)
    {
        componentRegister[name] = builder;
    }
}

std::shared_ptr<class Actor> Register::spawnActor(const std::string &typeName, class Scene *scene,
                                                  const std::string &actorName)
{
    try
    {
        return actorRegister.at(typeName)(scene, actorName);
    }
    catch (std::out_of_range e)
    {
        spdlog::error("Register: attempt to spawn actor type '{0}' not found in register", typeName);
        return nullptr;
    }
}

std::shared_ptr<class Component> Register::attachComponent(const std::string &typeName, class Actor *actor,
                                                           const std::string &componentName)
{
    try
    {
        return actor->attachComponent(componentRegister.at(typeName)(), componentName);
    }
    catch (std::out_of_range e)
    {
        spdlog::error("Register: attempt to create component type '{0}' not found in register", typeName);
        return nullptr;
    }
}

} // namespace cmx
