#ifndef CMX_PHYSICS_COMPONENT
#define CMX_PHYSICS_COMPONENT

// cmx
#include "cmx_component.h"
#include "cmx_physics.h"

#define MASK_ALL 0b11111111

// std
#include <memory>

namespace cmx
{

class PhysicsComponent : public Component
{
  public:
    PhysicsComponent();
    ~PhysicsComponent();

    void render(const class FrameInfo &, vk::PipelineLayout) override;

    void onDetach() override;
    void onAttach() override;

    void setPhysicsMode(PhysicsMode);

    PhysicsMode getPhysicsMode()
    {
        return _physicsMode;
    }

    float getAbsorptionCoefficient()
    {
        return _absorptionCoefficient;
    }

    const std::shared_ptr<class Shape> getShape()
    {
        return _shape;
    }

    void setShape(const std::string &);

    void setMask(uint8_t mask);

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &, tinyxml2::XMLElement *) override;
    void load(tinyxml2::XMLElement *) override;
    void editor(int i) override;

  protected:
    uint8_t _mask{MASK_ALL};
    PhysicsMode _physicsMode{PhysicsMode::STATIC};
    float _absorptionCoefficient{0.2f};
    std::shared_ptr<class Shape> _shape;
};

} // namespace cmx

#endif
