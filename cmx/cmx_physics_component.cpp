#include "cmx_physics_component.h"

// cmx
#include "cmx_actor.h"
#include "cmx_physics_manager.h"

namespace cmx
{

PhysicsComponent::PhysicsComponent()
{
    _renderZ = 0;
}

void PhysicsComponent::onDetach()
{
}

void PhysicsComponent::onAttach()
{
    getScene()->getPhysicsManager()->add(shared_from_this());
}

void PhysicsComponent::propagatePosition(const glm::vec3 &position)
{
    if (auto parent = _parent.lock())
    {
        parent->transform.position = position - _transform.position;
    }
}

void PhysicsComponent::setStatic()
{
    _physicsMode = PhysicsMode::STATIC;
    getScene()->getPhysicsManager()->add(shared_from_this());
}

void PhysicsComponent::setDynamic()
{
    _physicsMode = PhysicsMode::DYNAMIC;
    getScene()->getPhysicsManager()->add(shared_from_this());
}

void PhysicsComponent::setRigid()
{
    _physicsMode = PhysicsMode::RIGID;
    getScene()->getPhysicsManager()->add(shared_from_this());
}

} // namespace cmx
