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

    const std::shared_ptr<class Shape> getShape()
    {
        return _shape;
    }

    void setShape(const std::string &);
    void setMask(uint8_t mask);

    glm::mat3 getInverseInertiaTensorLocalSpace() const;
    glm::mat3 getInverseInertiaTensorWorldSpace() const;

    glm::vec3 getCenterOfMassLocalSpace() const;
    glm::vec3 getCenterOfMassWorldSpace() const;

    // rigid body functions BEGIN
    bool isRigid() const
    {
        return _physicsMode == PhysicsMode::RIGID;
    }
    void applyCollision(float dt, const class HitInfo &hitInfo, const PhysicsComponent &other);
    void applyImpulse(const glm::vec3 &impulseOrigin, const glm::vec3 &impulse);
    void applyImpulseLinear(const glm::vec3 &);
    void applyImpulseAngular(const glm::vec3 &);
    void applyGravity(float dt);
    void applyVelocity(float dt);
    // rigid body functions END

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &, tinyxml2::XMLElement *) override;
    void load(tinyxml2::XMLElement *) override;
    void editor(int i) override;

  protected:
    glm::vec3 _linearVelocity{0.f};
    glm::vec3 _angularVelocity{0.f};
    glm::vec3 _gravity{0.f, 10.f, 0.f};

    float _inverseMass{0.f};
    float _bounciness{0.5f};
    float _friction{0.5f};

    uint8_t _mask{MASK_ALL};
    PhysicsMode _physicsMode{PhysicsMode::STATIC};
    std::shared_ptr<class Shape> _shape;
};

} // namespace cmx

#endif
