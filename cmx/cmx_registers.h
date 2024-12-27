#ifndef CMX_REGISTERS
#define CMX_REGISTERS

#include "cmx_actor.h"
#include "cmx_mesh_actor.h"
#include "cmx_scene.h"
#include "cmx_viewport_actor.h"
#include "demo/rotating_actor.h"
#include <cstdlib>
#include <memory>

namespace cmx
{

namespace reg
{

inline const char *list[] = {"cmx::Actor", "cmx::ViewportActor", "cmx::MeshActor", "RotatingActor"};

inline std::shared_ptr<Actor> loadActor(const std::string &classname, Scene *scene, const std::string &name)
{
    if (classname.compare("cmx::Actor") == 0)
    {
        return Actor::spawn<Actor>(scene, name);
    }
    if (classname.compare("cmx::ViewportActor") == 0)
    {
        return Actor::spawn<ViewportActor>(scene, name);
    }
    if (classname.compare("cmx::MeshActor") == 0)
    {
        return Actor::spawn<MeshActor>(scene, name);
    }
    if (classname.compare("RotatingActor") == 0)
    {
        return Actor::spawn<RotatingActor>(scene, name);
    }

    spdlog::critical("Register: missing handle for {0}", classname);
    std::exit(EXIT_FAILURE);

    return Actor::spawn<Actor>(scene, name);
}

} // namespace reg

} // namespace cmx

#endif
