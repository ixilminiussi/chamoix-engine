#ifndef CMX_PHYSICS_COMPONENT
#define CMX_PHYSICS_COMPONENT

// cmx
#include "cmx_component.h"
#include "cmx_physics.h"

// std
#include <memory>

namespace cmx
{

class PhysicsComponent : public Component
{
  public:
    PhysicsComponent();

    void onDetach() override;
    void onAttach() override;

    void setStatic();
    void setDynamic();
    void setRigid();

    void propagatePosition(const glm::vec3 &position);

    PhysicsMode getPhysicsMode()
    {
        return _physicsMode;
    }

    float getAbsorptionCoefficient()
    {
        return _absorptionCoefficient;
    }

  protected:
    PhysicsMode _physicsMode{PhysicsMode::STATIC};
    float _absorptionCoefficient{0.2f};
};

} // namespace cmx

#endif
