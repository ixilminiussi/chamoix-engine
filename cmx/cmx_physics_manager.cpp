#include "cmx_physics_manager.h"

// cmx
#include "cmx_physics_actor.h"
#include "cmx_physics_component.h"
#include <glm/ext/scalar_constants.hpp>
#include <memory>

namespace cmx
{

PhysicsManager::PhysicsManager() : _dynamicComponents{}, _staticComponents{}
{
}

PhysicsManager::~PhysicsManager()
{
}

void PhysicsManager::executeStep(float dt)
{
    auto it = _rigidComponents.begin();
    while (it != _rigidComponents.end())
    {
        std::shared_ptr<PhysicsComponent> component = *it;

        if (PhysicsActor *parent = dynamic_cast<PhysicsActor *>(component->getParent()))
        {
            Transform transform = component->getAbsoluteTransform();

            if (transform.position.y + transform.scale.y < _floor)
            {
                parent->addVelocity(glm::vec3{0.f, _gravity, 0.f});
            }
            parent->applyVelocity(dt);

            transform = component->getAbsoluteTransform();
            if (transform.position.y + transform.scale.y >= _floor)
            {
                transform.position.y = _floor - transform.scale.y;
                component->propagatePosition(transform.position);

                glm::vec3 newVelocity = parent->getVelocity();
                newVelocity.y *= -component->getAbsorptionCoefficient();
                parent->setVelocity(newVelocity);
            }
        }
        else
        {
            spdlog::warn("PhysicsManager: Parent actor of RigidBodyComponent is not a PhysicsActor and therefore "
                         "cannot be moved");
        }
    }
}

void PhysicsManager::add(std::shared_ptr<Component> component)
{
    if (std::shared_ptr<PhysicsComponent> physicsComponent = std::dynamic_pointer_cast<PhysicsComponent>(component))
    {
        switch (physicsComponent->getPhysicsMode())
        {
        case PhysicsMode::RIGID:
            moveToRigid(physicsComponent);
            break;
        case PhysicsMode::STATIC:
            moveToStatic(physicsComponent);
            break;
        case PhysicsMode::DYNAMIC:
            moveToDynamic(physicsComponent);
            break;
        }
    }
    else
    {
        spdlog::error("PhysicsManager: Attempt at adding invalid component '{0}' to manager. (Cannot be cast to "
                      "Physics Component)",
                      component->name);
    }
}

void PhysicsManager::remove(std::shared_ptr<PhysicsComponent> physicsComponent)
{
    switch (physicsComponent->getPhysicsMode())
    {
    case PhysicsMode::RIGID:
        _rigidComponents.erase(physicsComponent);
        break;
    case PhysicsMode::STATIC:
        _staticComponents.erase(physicsComponent);
        break;
    case PhysicsMode::DYNAMIC:
        _dynamicComponents.erase(physicsComponent);
        break;
    }
}

void PhysicsManager::moveToDynamic(std::shared_ptr<PhysicsComponent> component)
{
    _dynamicComponents.insert(component);
    _staticComponents.erase(component);
    _rigidComponents.erase(component);
}

void PhysicsManager::moveToStatic(std::shared_ptr<PhysicsComponent> component)
{
    _staticComponents.insert(component);
    _dynamicComponents.erase(component);
    _rigidComponents.erase(component);
}

void PhysicsManager::moveToRigid(std::shared_ptr<PhysicsComponent> component)
{
    _rigidComponents.insert(component);
    _dynamicComponents.erase(component);
    _staticComponents.erase(component);
}

} // namespace cmx
