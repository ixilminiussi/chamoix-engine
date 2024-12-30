#include "cmx_register.h"

// cmx
#include "cmx_actor.h"
#include "cmx_camera_component.h"
#include "cmx_component.h"
#include "cmx_mesh_actor.h"
#include "cmx_mesh_component.h"
#include "cmx_viewport_actor.h"
#include "cmx_viewport_ui_component.h"
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
    actorRegister["cmx::ViewportActor"] = [](class Scene *scene, const std::string &name) {
        return Actor::spawn<ViewportActor>(scene, name);
    };

    componentRegister["cmx::Component"] = [](class Actor *actor, const std::string &name) {
        std::shared_ptr<Component> component = std::make_shared<Component>();
        actor->attachComponent(component);
        return component;
    };
    componentRegister["cmx::MeshComponent"] = [](class Actor *actor, const std::string &name) {
        std::shared_ptr<Component> meshComponent = std::make_shared<MeshComponent>();
        actor->attachComponent(meshComponent);
        return meshComponent;
    };
    componentRegister["cmx::CameraComponent"] = [](class Actor *actor, const std::string &name) {
        std::shared_ptr<Component> cameraComponent = std::make_shared<CameraComponent>();
        actor->attachComponent(cameraComponent);
        return cameraComponent;
    };
    componentRegister["cmx::ViewportUIComponent"] = [](class Actor *actor, const std::string &name) {
        std::shared_ptr<Component> viewportUIComponent = std::make_shared<ViewportUIComponent>();
        actor->attachComponent(viewportUIComponent);
        return viewportUIComponent;
    };
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

void Register::addComponent(std::string name,
                            std::function<std::shared_ptr<class Component>(class Actor *, const std::string &)> builder)
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
        return componentRegister.at(typeName)(actor, componentName);
    }
    catch (std::out_of_range e)
    {
        spdlog::error("Register: attempt to create component type '{0}' not found in register", typeName);
        return nullptr;
    }
}

} // namespace cmx
