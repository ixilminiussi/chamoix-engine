#ifndef CMX_PHYSICS
#define CMX_PHYSICS

#include <string>
namespace cmx
{

enum PhysicsMode
{
    STATIC,
    DYNAMIC,
    RIGID
};

constexpr const char *physicsModeToString(PhysicsMode type)
{
    switch (type)
    {
    case PhysicsMode::STATIC:
        return "Static";
    case PhysicsMode::DYNAMIC:
        return "Dynamic";
    case PhysicsMode::RIGID:
        return "Rigid";
    default:
        return "Unknown";
    }
}

} // namespace cmx

#endif
