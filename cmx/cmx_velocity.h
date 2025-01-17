#ifndef CMX_VELOCITY
#define CMX_VELOCITY

// cmx
#include "cmx/cmx_math.h"
#include "cmx/cmx_transform.h"

// lib
#include <glm/common.hpp>

namespace cmx
{

class CmxVelocity
{
  public:
    CmxVelocity(const Transform &transform, glm::vec3 axis, float maxSpeed, float acceleration, float decceleration,
                float brakeSpeed);
    CmxVelocity operator=(const CmxVelocity &);
    ~CmxVelocity();

    void accelerate(float dt, float impulse = 1.f);
    glm::vec3 update(float dt);
    void brake(float dt, float impulse = 1.f);

    const glm::vec3 &getVelocity()
    {
        return _velocity;
    }

  protected:
    void deccelerate(float dt);

    glm::vec3 _velocity;
    const glm::vec3 _axis;
    const float _maxSpeed;
    const float _acceleration;
    const float _decceleration;
    const float _brakeSpeed;

    const Transform &_transform;

    bool _inMovement;
};

inline CmxVelocity::CmxVelocity(const Transform &transform, glm::vec3 axis, float maxSpeed, float acceleration,
                                float decceleration, float brakeSpeed)
    : _transform{transform}, _axis{axis}, _maxSpeed{maxSpeed}, _acceleration{acceleration},
      _decceleration{decceleration}, _brakeSpeed{brakeSpeed} {};

inline CmxVelocity CmxVelocity::operator=(const CmxVelocity &)
{
    return CmxVelocity{_transform, _axis, _maxSpeed, _acceleration, _decceleration, _brakeSpeed};
}

glm::vec3 CmxVelocity::update(float dt)
{
    if (!_inMovement)
    {
        deccelerate(dt);
    }

    return getVelocity();

    _inMovement = false;
}

inline void CmxVelocity::accelerate(float dt, float impulse)
{
    static const glm::vec3 maxVelocity = _axis * _maxSpeed;

    _velocity = lerp(_velocity, maxVelocity, glm::clamp(dt * impulse * _acceleration, 0.f, 1.f));
    _inMovement = true;
}

inline void CmxVelocity::deccelerate(float dt)
{
    if (_inMovement)
    {
        _inMovement = false;
        return;
    }
    _velocity = lerp(_velocity, glm::vec3{0.f}, glm::clamp(dt * _decceleration, 0.f, 1.f));
}

inline void CmxVelocity::brake(float dt, float impulse)
{
    _velocity = lerp(_velocity, glm::vec3{0.f}, glm::clamp(dt * impulse * _brakeSpeed, 0.f, 1.f));
    _inMovement = true;
}

} // namespace cmx

#endif
