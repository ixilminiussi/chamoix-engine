#include "cmx_physics_manager.h"

// cmx
#include "cmx_physics_actor.h"
#include "cmx_physics_body.h"
#include "cmx_shapes.h"

// lib
#include <glm/ext/scalar_constants.hpp>

// std
#include <memory>
#include <ranges>

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
    std::vector<PhysicsBody *> flatAll;
    flatAll.reserve(_rigidBodies.size() + _dynamicBodies.size() + _staticBodies.size());

    flatAll.insert(flatAll.end(), _rigidBodies.begin(), _rigidBodies.end());
    flatAll.insert(flatAll.end(), _dynamicBodies.begin(), _dynamicBodies.end());
    flatAll.insert(flatAll.end(), _staticBodies.begin(), _staticBodies.end());

    bool first = true;

    for (auto it = _rigidBodies.begin(); it != _rigidBodies.end(); it++)
    {
        (*it)->applyGravity(dt);
        (*it)->applyVelocity(dt);
    }

    int innerAll = _rigidBodies.size() + _dynamicBodies.size();

    for (int i = 0; i < innerAll; i++)
    {
        PhysicsBody *physicsBody = flatAll[i];
        std::shared_ptr<Shape> shape = physicsBody->getShape();

        if (shape.get() == nullptr)
            continue;

        if (first)
        {
            shape->swapBuffer();
        }

        for (int j = i + 1; j < flatAll.size(); j++)
        {
            PhysicsBody *otherBody = flatAll[j];
            std::shared_ptr<Shape> otherShape = otherBody->getShape();

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
                    physicsBody->applyCollision(dt, hitInfo, *otherBody);
                }

                if ((otherBody)->isRigid())
                {
                    hitInfo.flip();
                    (otherBody)->applyCollision(dt, hitInfo, *physicsBody);
                    hitInfo.flip();
                }

                shape->addOverlappingComponent(otherBody);
                otherShape->addOverlappingComponent(physicsBody);

                if (auto parent = dynamic_cast<PhysicsActor *>(physicsBody->getParentActor()))
                {
                    parent->onContinuousOverlap(physicsBody, otherBody, otherBody->getParentActor(), hitInfo);
                }
                if (auto parent = dynamic_cast<PhysicsActor *>(otherBody->getParentActor()))
                {
                    hitInfo.flip();
                    parent->onContinuousOverlap(otherBody, physicsBody, physicsBody->getParentActor(), hitInfo);
                }

                if (!shape->wasOverlapping(otherBody))
                {
                    if (auto parent = dynamic_cast<PhysicsActor *>(physicsBody->getParentActor()))
                    {
                        parent->onBeginOverlap(physicsBody, otherBody, otherBody->getParentActor(), hitInfo);
                    }
                    if (auto parent = dynamic_cast<PhysicsActor *>(otherBody->getParentActor()))
                    {
                        hitInfo.flip();
                        parent->onBeginOverlap(otherBody, physicsBody, physicsBody->getParentActor(), hitInfo);
                    }
                }
            }
            else
            {
                if (shape->wasOverlapping(otherBody))
                {
                    if (auto parent = dynamic_cast<PhysicsActor *>(physicsBody->getParentActor()))
                    {
                        parent->onEndOverlap(physicsBody, otherBody, otherBody->getParentActor());
                    }
                    if (auto parent = dynamic_cast<PhysicsActor *>(otherBody->getParentActor()))
                    {
                        parent->onEndOverlap(otherBody, physicsBody, physicsBody->getParentActor());
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
