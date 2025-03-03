#include "cmx_physics_manager.h"

// cmx
#include "cmx_physics_actor.h"
#include "cmx_physics_body.h"
#include "cmx_shapes.h"

// lib
#include <glm/ext/scalar_constants.hpp>

// std
#include <memory>

namespace cmx
{

PhysicsManager::PhysicsManager() : _dynamicBodies{}, _staticBodies{}, _rigidBodies{}
{
}

PhysicsManager::~PhysicsManager()
{
}

void PhysicsManager::executeStep(float dt)
{
    auto advance = [&](auto &it) {
        it++;

        if (it == _rigidBodies.end())
        {
            it = _dynamicBodies.begin();
        }
        if (it == _dynamicBodies.end())
        {
            it = _staticBodies.begin();
        }
    };

    bool first = true;

    for (auto it = _rigidBodies.begin(); it != _rigidBodies.end(); it++)
    {
        (*it)->applyGravity(dt);
        (*it)->applyVelocity(dt);
        it++;
    }

    for (auto it = (_rigidBodies.size() > 0) ? _rigidBodies.begin() : _dynamicBodies.begin();
         it != _dynamicBodies.end() && it != _staticBodies.begin(); advance(it))
    {
        PhysicsBody *physicsBody = *it;
        std::shared_ptr<Shape> shape = physicsBody->getShape();

        if (shape.get() == nullptr)
            continue;

        if (first)
        {
            shape->swapBuffer();
        }

        auto itAlt = it;

        advance(itAlt);

        for (; itAlt != _staticBodies.end(); advance(itAlt))
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
                if (physicsBody->isRigid())
                {
                    physicsBody->applyCollision(dt, hitInfo, **itAlt);
                }

                if ((*itAlt)->isRigid())
                {
                    hitInfo.flip();
                    (*itAlt)->applyCollision(dt, hitInfo, *physicsBody);
                    hitInfo.flip();
                }

                shape->addOverlappingComponent(*itAlt);
                otherShape->addOverlappingComponent(physicsBody);

                if (auto parent = dynamic_cast<PhysicsActor *>(physicsBody->getParentActor()))
                {
                    parent->onContinuousOverlap(physicsBody, *itAlt, (*itAlt)->getParentActor(), hitInfo);
                }
                if (auto parent = dynamic_cast<PhysicsActor *>((*itAlt)->getParentActor()))
                {
                    hitInfo.flip();
                    parent->onContinuousOverlap(*itAlt, physicsBody, physicsBody->getParentActor(), hitInfo);
                }

                if (!shape->wasOverlapping(*itAlt))
                {
                    if (auto parent = dynamic_cast<PhysicsActor *>(physicsBody->getParentActor()))
                    {
                        parent->onBeginOverlap(physicsBody, *itAlt, (*itAlt)->getParentActor(), hitInfo);
                    }
                    if (auto parent = dynamic_cast<PhysicsActor *>((*itAlt)->getParentActor()))
                    {
                        hitInfo.flip();
                        parent->onBeginOverlap(*itAlt, physicsBody, physicsBody->getParentActor(), hitInfo);
                    }
                }
            }
            else
            {
                if (shape->wasOverlapping(*itAlt))
                {
                    if (auto parent = dynamic_cast<PhysicsActor *>(physicsBody->getParentActor()))
                    {
                        parent->onEndOverlap(physicsBody, *itAlt, (*itAlt)->getParentActor());
                    }
                    if (auto parent = dynamic_cast<PhysicsActor *>((*itAlt)->getParentActor()))
                    {
                        parent->onEndOverlap(*itAlt, physicsBody, physicsBody->getParentActor());
                    }
                }
            }
        }

        first = false;
    }
}

void PhysicsManager::add(PhysicsBody *body)
{
    switch (body->getPhysicsMode())
    {
    case PhysicsMode::RIGID:
        moveToRigid(body);
        break;
    case PhysicsMode::STATIC:
        moveToStatic(body);
        break;
    case PhysicsMode::DYNAMIC:
        moveToDynamic(body);
        break;
    }
}

void PhysicsManager::remove(PhysicsBody *body)
{
    switch (body->getPhysicsMode())
    {
    case PhysicsMode::RIGID:
        _rigidBodies.erase(body);
        break;
    case PhysicsMode::STATIC:
        _staticBodies.erase(body);
        break;
    case PhysicsMode::DYNAMIC:
        _dynamicBodies.erase(body);
        break;
    }
}

void PhysicsManager::moveToDynamic(PhysicsBody *body)
{
    _dynamicBodies.insert(body);
    _staticBodies.erase(body);
    _rigidBodies.erase(body);
}

void PhysicsManager::moveToStatic(PhysicsBody *body)
{
    _staticBodies.insert(body);
    _dynamicBodies.erase(body);
    _rigidBodies.erase(body);
}

void PhysicsManager::moveToRigid(PhysicsBody *body)
{
    _rigidBodies.insert(body);
    _dynamicBodies.erase(body);
    _staticBodies.erase(body);
}

} // namespace cmx
