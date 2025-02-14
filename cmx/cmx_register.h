#ifndef CMX_REGISTER
#define CMX_REGISTER

// cmx
#include "cmx_actor.h"

// std
#include <cstdlib>
#include <memory>
#include <unordered_map>

namespace cmx
{

class Register
{
  public:
    static Register *getInstance();

    void addActor(std::string, std::function<class Actor *(class Scene *, const std::string &)>);
    void addComponent(std::string, std::function<std::shared_ptr<class Component>()>);

    class Actor *spawnActor(const std::string &, class Scene *, const std::string &);
    std::shared_ptr<class Component> attachComponent(const std::string &, class Actor *, const std::string &,
                                                     bool force = false);

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

    std::unordered_map<std::string, std::function<class Actor *(class Scene *, const std::string &)>> actorRegister{};

    std::unordered_map<std::string, std::function<std::shared_ptr<class Component>()>> componentRegister{};

    static Register *_instance;
};

} // namespace cmx

#endif
