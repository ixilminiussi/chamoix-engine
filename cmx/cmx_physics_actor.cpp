#include "cmx_physics_actor.h"

// cmx
#include "cmx_physics_component.h"
#include <glm/ext/scalar_constants.hpp>
#include <glm/geometric.hpp>

namespace cmx
{

void PhysicsActor::onBegin()
{
    Actor::onBegin();

    _physicsComponent = std::make_shared<PhysicsComponent>();
    attachComponent(_physicsComponent);
}

void PhysicsActor::setVelocity(glm::vec3 velocity)
{
    _velocity = velocity;
}

void PhysicsActor::addVelocity(glm::vec3 force)
{
    _velocity += force;
}

void PhysicsActor::applyVelocity(float dt)
{
    transform.position += _velocity * dt;
}

glm::vec3 PhysicsActor::getNormalizedVelocity()
{
    if (_velocity.length() <= glm::epsilon<float>())
        return glm::vec3{0.f, 0.f, 0.f};

    return glm::normalize(_velocity);
}

} // namespace cmx
