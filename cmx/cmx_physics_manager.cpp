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

PhysicsManager::PhysicsManager() : _dynamicComponents{}, _staticComponents{}, _rigidComponents{}
{
}

PhysicsManager::~PhysicsManager()
{
}

void PhysicsManager::executeStep(float dt)
{
    auto advance = [&](auto &it) {
        it++;
        if (it == _rigidComponents.end())
        {
            it = _dynamicComponents.begin();
        }
        if (it == _dynamicComponents.end())
        {
            it = _staticComponents.begin();
        }
    };

    bool first = true;

    for (auto it = _rigidComponents.begin(); it != _rigidComponents.end(); it++)
    {
        (*it)->applyGravity(dt);
        (*it)->applyVelocity(dt);
    }

    auto it = (_rigidComponents.size() > 0) ? _rigidComponents.begin() : _dynamicComponents.begin();
    for (; it != _dynamicComponents.end() && it != _staticComponents.begin(); advance(it))
    {
        std::shared_ptr<PhysicsComponent> physicsComponent = *it;
        std::shared_ptr<Shape> shape = physicsComponent->getShape();

        if (shape.get() == nullptr)
            continue;

        if (first)
        {
            shape->swapBuffer();
        }

        auto itAlt = it;

        advance(itAlt);

        for (; itAlt != _staticComponents.end(); advance(itAlt))
        {
            std::shared_ptr<Shape> otherShape = (*itAlt)->getShape();

            if (otherShape.get() == nullptr)
                continue;

            if (first)
            {
                otherShape->swapBuffer();
            }

            HitInfo hitInfo{};
            if (shape->overlapsWith(*otherShape, hitInfo))
            {
                if (physicsComponent->isRigid())
                {
                    physicsComponent->applyCollision(dt, hitInfo, **itAlt);
                }

                if ((*itAlt)->isRigid())
                {
                    hitInfo.flip();
                    (*itAlt)->applyCollision(dt, hitInfo, *physicsComponent);
                    hitInfo.flip();
                }

                shape->addOverlappingComponent(itAlt->get());
                otherShape->addOverlappingComponent(physicsComponent.get());

                if (!shape->wasOverlapping(itAlt->get()))
                {
                    if (auto parent = dynamic_cast<PhysicsActor *>(physicsComponent->getParent()))
                    {
                        parent->onBeginOverlap(physicsComponent.get(), itAlt->get(), (*itAlt)->getParent(), hitInfo);
                    }
                    if (auto parent = dynamic_cast<PhysicsActor *>((*itAlt)->getParent()))
                    {
                        hitInfo.flip();
                        parent->onBeginOverlap(itAlt->get(), physicsComponent.get(), physicsComponent->getParent(),
                                               hitInfo);
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
