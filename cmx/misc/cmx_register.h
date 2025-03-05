#ifndef CMX_REGISTER
#define CMX_REGISTER

#include "cmx_actor.h"
#include "cmx_component.h"

// std
#include <cstdlib>
#include <functional>
#include <memory>
#include <unordered_map>

namespace cmx
{

class Register
{
  public:
    static Register &getInstance();

    void addActor(const char *, std::function<class Actor *(class Scene *, const char *)>);
    void addComponent(const char *, std::function<std::shared_ptr<class Component>()>);
    void addMaterial(const char *, std::function<class Material *()>);

    class Actor *spawnActor(const char *, class Scene *, const char *);
    std::shared_ptr<class Component> attachComponent(const char *, class Actor *, const char *, bool force = false);

    class Material *getMaterial(const char *);

    const auto &getActorRegister()
    {
        return actorRegister;
    }

    const auto &getComponentRegister()
    {
        return componentRegister;
    }

  private:
    Register();
    ~Register();

    std::map<std::string, std::function<class Actor *(class Scene *, const char *)>> actorRegister;

    std::map<std::string, std::function<std::shared_ptr<class Component>()>> componentRegister;

    std::map<std::string, std::function<class Material *()>> materialRegister;
};

} // namespace cmx

#endif
