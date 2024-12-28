#ifndef CMX_REGISTER
#define CMX_REGISTER

#include "cmx_actor.h"
#include <cstdlib>
#include <memory>
#include <unordered_map>

namespace cmx
{

class Register
{
  public:
    static Register *getInstance();

    std::unordered_map<std::string, std::function<std::shared_ptr<Actor>(class Scene *, const std::string &)>>
        actorsRegister{};

  private:
    Register();
    ~Register();

    static Register *_instance;
};

} // namespace cmx

#endif
