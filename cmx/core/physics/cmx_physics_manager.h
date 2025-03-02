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

    void add(class PhysicsBody *);
    void remove(class PhysicsBody *);

  private:
    void moveToDynamic(class PhysicsBody *);
    void moveToStatic(class PhysicsBody *);
    void moveToRigid(class PhysicsBody *);

    std::set<class PhysicsBody *> _rigidBodies;
    std::set<class PhysicsBody *> _dynamicBodies;
    std::set<class PhysicsBody *> _staticBodies;

    float _gravity = .5f;
    float _floor = 10.0f;
};

} // namespace cmx

#endif
