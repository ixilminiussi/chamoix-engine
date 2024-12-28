#include "cmx_register.h"

// cmx
#include "cmx_actor.h"
#include "cmx_mesh_actor.h"
#include "cmx_viewport_actor.h"

namespace cmx
{

Register *Register::_instance{};

Register::Register()
{
    actorsRegister["cmx::Actor"] = [](class Scene *scene, const std::string &name) {
        return Actor::spawn<Actor>(scene, name);
    };
    actorsRegister["cmx::MeshActor"] = [](class Scene *scene, const std::string &name) {
        return Actor::spawn<MeshActor>(scene, name);
    };
    actorsRegister["cmx::ViewportActor"] = [](class Scene *scene, const std::string &name) {
        return Actor::spawn<ViewportActor>(scene, name);
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

} // namespace cmx
