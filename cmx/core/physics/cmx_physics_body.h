#ifndef CMX_PHYSICS_BODY
#define CMX_PHYSICS_BODY

// cmx
#include "cmx_physics.h"
#include "cmx_transform.h"

// std
#include <memory>

#define MASK_ALL 0b11111111

namespace cmx
{

class PhysicsBody : public virtual Transformable
{
  public:
    PhysicsBody(class Actor **parentP);
    virtual ~PhysicsBody() = default;

    Actor *getParentActor() const
    {
        return *_parentP;
    };

    tinyxml2::XMLElement &save(tinyxml2::XMLElement &) const;
    void load(tinyxml2::XMLElement *);
    void editor(int i);

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

    glm::vec3 getLinearVelocity() const
    {
        return _linearVelocity;
    }
    void setLinearVelocity(const glm::vec3 &velocity)
    {
        _linearVelocity = velocity;
    }

    // rigid body functions BEGIN
    bool isRigid() const
    {
        return _physicsMode == PhysicsMode::RIGID;
    }
    void applyCollision(float dt, const struct HitInfo &hitInfo, const PhysicsBody &other);
    void applyImpulse(const glm::vec3 &impulseOrigin, const glm::vec3 &impulse);
    void applyImpulseLinear(const glm::vec3 &);
    void applyImpulseAngular(const glm::vec3 &);
    void applyGravity(float dt);
    void applyVelocity(float dt);

    void setMass(float mass);
    void setInverseMass(float inverseMass);
    // rigid body functions END

  private:
    glm::vec3 _linearVelocity{0.f};
    glm::vec3 _angularVelocity{0.f};
    glm::vec3 _gravity{0.f, 10.f, 0.f};

    float _inverseMass{0.f};
    float _bounciness{0.5f};
    float _airResistance{1.f};
    float _friction{0.5f};

    Actor **_parentP{nullptr};

    uint8_t _mask{MASK_ALL};
    PhysicsMode _physicsMode{PhysicsMode::STATIC};
    std::shared_ptr<class Shape> _shape;
};

} // namespace cmx

#endif
