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
    ~PhysicsComponent();

    void render(const class FrameInfo &, VkPipelineLayout) override;

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

    const std::shared_ptr<class CmxShape> getShape()
    {
        return _cmxShape;
    }

    void setShape(const std::string &);

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &, tinyxml2::XMLElement *) override;
    void load(tinyxml2::XMLElement *) override;
    void editor(int i) override;

  protected:
    PhysicsMode _physicsMode{PhysicsMode::STATIC};
    float _absorptionCoefficient{0.2f};
    std::shared_ptr<class CmxShape> _cmxShape;
};

} // namespace cmx

#endif
