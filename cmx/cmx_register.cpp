#include "cmx_register.h"

// cmx
#include "cmx_camera_component.h"
#include "cmx_hud_component.h"
#include "cmx_mesh_actor.h"
#include "cmx_physics_actor.h"
#include "cmx_physics_component.h"
#include "cmx_point_light_actor.h"
#include "cmx_point_light_component.h"

// lib
#include <spdlog/spdlog.h>

// std
#include <memory>
#include <stdexcept>

namespace cmx
{

Register::Register() : actorRegister{}, componentRegister{}
{
}

Register::~Register()
{
}

Register &Register::getInstance()
{
    static Register instance;
    return instance;
}

void Register::addActor(std::string name, std::function<Actor *(class Scene *, const std::string &)> builder)
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

Actor *Register::spawnActor(const std::string &typeName, class Scene *scene, const std::string &actorName)
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
                                                           const std::string &componentName, bool force)
{
    try
    {
        return actor->attachComponent(componentRegister.at(typeName)(), componentName, force);
    }
    catch (std::out_of_range e)
    {
        spdlog::error("Register: attempt to create component type '{0}' not found in register", typeName);
        return nullptr;
    }
}

} // namespace cmx
