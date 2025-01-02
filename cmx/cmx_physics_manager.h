#ifndef CMX_PHYSICS_MANAGER
#define CMX_PHYSICS_MANAGER

// std
#include <memory>
#include <set>

namespace cmx
{

class PhysicsManager
{
  public:
    PhysicsManager();
    ~PhysicsManager();

    void executeStep(float dt);

    void add(std::shared_ptr<class Component>);
    void remove(std::shared_ptr<class PhysicsComponent>);

  private:
    void moveToDynamic(std::shared_ptr<class PhysicsComponent>);
    void moveToStatic(std::shared_ptr<class PhysicsComponent>);
    void moveToRigid(std::shared_ptr<class PhysicsComponent>);

    std::set<std::shared_ptr<class PhysicsComponent>> _rigidComponents;
    std::set<std::shared_ptr<class PhysicsComponent>> _dynamicComponents;
    std::set<std::shared_ptr<class PhysicsComponent>> _staticComponents;

    float _gravity = .89f;
    float _floor = 10.0f;
};

} // namespace cmx

#endif
