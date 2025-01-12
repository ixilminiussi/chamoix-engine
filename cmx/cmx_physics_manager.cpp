#include "cmx_physics_manager.h"

// cmx
#include "cmx_physics_actor.h"
#include "cmx_physics_component.h"
#include "cmx_shapes.h"

// lib
#include <glm/ext/scalar_constants.hpp>

// std
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
    auto advance = [&](auto &it) {
        it++;
        if (it == _dynamicComponents.end())
        {
            it = _staticComponents.begin();
        }
    };

    bool first = true;

    for (auto it = _dynamicComponents.begin(); it != _dynamicComponents.end(); it++)
    {
        std::shared_ptr<PhysicsComponent> physicsComponent = *it;
        std::shared_ptr<CmxShape> shape = physicsComponent->getShape();

        if (shape.get() == nullptr)
            continue;

        if (first)
        {
            shape->updateDimensions();
            shape->swapBuffer();
        }

        auto itAlt = it;

        advance(itAlt);

        for (; itAlt != _staticComponents.end(); advance(itAlt))
        {
            std::shared_ptr<CmxShape> otherShape = (*itAlt)->getShape();

            if (otherShape.get() == nullptr)
                continue;

            if (first)
            {
                otherShape->updateDimensions();
                otherShape->swapBuffer();
            }

            if (shape->overlapsWith(*otherShape))
            {
                shape->addOverlappingComponent(itAlt->get());
                otherShape->addOverlappingComponent(physicsComponent.get());

                if (!shape->wasOverlapping(itAlt->get()))
                {
                    if (auto parent = dynamic_cast<PhysicsActor *>(physicsComponent->getParent()))
                    {
                        parent->onBeginOverlap(physicsComponent.get(), itAlt->get(), (*itAlt)->getParent());
                    }
                    if (auto parent = dynamic_cast<PhysicsActor *>((*itAlt)->getParent()))
                    {
                        parent->onBeginOverlap(itAlt->get(), physicsComponent.get(), physicsComponent->getParent());
                    }
                }
            }
            else
            {
                if (shape->wasOverlapping(itAlt->get()))
                {
                    if (auto parent = dynamic_cast<PhysicsActor *>(physicsComponent->getParent()))
                    {
                        parent->onEndOverlap(physicsComponent.get(), itAlt->get(), (*itAlt)->getParent());
                    }
                    if (auto parent = dynamic_cast<PhysicsActor *>((*itAlt)->getParent()))
                    {
                        parent->onEndOverlap(itAlt->get(), physicsComponent.get(), physicsComponent->getParent());
                    }
                }
            }
        }

        first = false;
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
