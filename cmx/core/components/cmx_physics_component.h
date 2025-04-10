#ifndef CMX_PHYSICS_COMPONENT
#define CMX_PHYSICS_COMPONENT

// cmx
#include "cmx_component.h"
#include "cmx_drawable.h"
#include "cmx_physics_body.h"
#include "cmx_register.h"

namespace cmx
{

class PhysicsComponent : public Component,
                         public virtual PhysicsBody
#ifndef NDEBUG
    ,
                         public virtual Drawable
#endif
{
  public:
    PhysicsComponent();
    ~PhysicsComponent() = default;

    CLONEABLE_COMPONENT(PhysicsComponent)

    void onDetach() override;
    void onAttach() override;

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &, tinyxml2::XMLElement *) const override;
    void load(tinyxml2::XMLElement *) override;
    void editor(int i) override;

    void setPhysicsMode(PhysicsMode) override;
    void setShape(const std::string &) override;
};

REGISTER_COMPONENT(cmx::PhysicsComponent)

} // namespace cmx

#endif
