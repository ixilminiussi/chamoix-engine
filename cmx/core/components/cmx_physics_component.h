#ifndef CMX_PHYSICS_COMPONENT
#define CMX_PHYSICS_COMPONENT

// cmx
#include "cmx_component.h"
#include "cmx_physics_body.h"
#include "cmx_register.h"

namespace cmx
{

class PhysicsComponent : public Component, public virtual PhysicsBody
{
  public:
    PhysicsComponent();
    ~PhysicsComponent() = default;

    CLONEABLE(PhysicsComponent)

    void onDetach() override;
    void onAttach() override;

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &, tinyxml2::XMLElement *) const override;
    void load(tinyxml2::XMLElement *) override;
    void editor(int i) override;
};

REGISTER_COMPONENT(cmx::PhysicsComponent)

} // namespace cmx

#endif
