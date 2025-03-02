#ifndef CMX_PHYSICS
#define CMX_PHYSICS

#include <cstring>
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

inline PhysicsMode physicsModeFromString(const std::string &string)
{
    if (strcmp(string.c_str(), "Rigid") == 0)
    {
        return PhysicsMode::RIGID;
    }
    if (strcmp(string.c_str(), "Dynamic") == 0)
    {
        return PhysicsMode::DYNAMIC;
    }
    return PhysicsMode::STATIC;
}

} // namespace cmx

#endif
