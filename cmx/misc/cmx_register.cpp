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

void Register::addActor(const char *name, std::function<Actor *(class Scene *, const char *)> builder)
{
    if (actorRegister.find(name) != actorRegister.end())
    {
        actorRegister.at(name);
        spdlog::warn("Register: duplicate actor '{0}' in register", name);
        return;
    }

    actorRegister[name] = builder;
}

void Register::addComponent(const char *name, std::function<std::shared_ptr<class Component>()> builder)
{
    if (componentRegister.find(name) != componentRegister.end())
    {
        componentRegister.at(name);
        spdlog::warn("Register: duplicate component '{0}' in register", name);
        return;
    }

    componentRegister[name] = builder;
}

void Register::addMaterial(const char *name, std::function<class Material *()> builder)
{
    if (materialRegister.find(name) != materialRegister.end())
    {
        materialRegister.at(name);
        spdlog::warn("Register: duplicate material '{0}' in register", name);
        return;
    }

    materialRegister[name] = builder;
}

Actor *Register::spawnActor(const char *typeName, class Scene *scene, const char *actorName)
{
    if (actorRegister.find(typeName) == actorRegister.end())
    {
        spdlog::error("Register: attempt to spawn actor type '{0}' not found in register", typeName);
        return nullptr;
    }

    return actorRegister[typeName](scene, actorName);
}

std::shared_ptr<class Component> Register::attachComponent(const char *typeName, class Actor *actor,
                                                           const char *componentName, bool force)
{
    if (componentRegister.find(typeName) == componentRegister.end())
    {
        spdlog::error("Register: attempt to create component type '{0}' not found in register", typeName);
        return nullptr;
    }

    return actor->attachComponent(componentRegister.at(typeName)(), componentName, force);
}

class Material *Register::getMaterial(const char *typeName)
{
    if (materialRegister.find(typeName) == materialRegister.end())
    {
        spdlog::error("Register: attempt to get material type '{0}' not found in register", typeName);
        return nullptr;
    }

    return materialRegister[typeName]();
}

} // namespace cmx
