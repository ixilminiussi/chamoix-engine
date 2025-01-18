#include "cmx_physics_component.h"

// cmx
#include "cmx/cmx_editor.h"
#include "cmx_actor.h"
#include "cmx_frame_info.h"
#include "cmx_physics_manager.h"
#include "cmx_primitives.h"
#include "cmx_render_system.h"
#include "cmx_shapes.h"

namespace cmx
{

PhysicsComponent::PhysicsComponent()
{
    _renderZ = DEBUG_BILLBOARD_Z;
    _requestedRenderSystem = EDGE_RENDER_SYSTEM;
}

PhysicsComponent::~PhysicsComponent()
{
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

void PhysicsComponent::setShape(const std::string &type)
{
    if (type.compare(PRIMITIVE_SPHERE) == 0)
    {
        _cmxShape = std::shared_ptr<CmxShape>(new CmxSphere(this));
        return;
    }
    if (type.compare(PRIMITIVE_CUBE) == 0)
    {
        _cmxShape = std::shared_ptr<CmxShape>(new CmxCuboid(this));
        return;
    }
    if (type.compare(PRIMITIVE_CONTAINER) == 0)
    {
        _cmxShape = std::shared_ptr<CmxShape>(new CmxContainer(this));
        return;
    }

    spdlog::warn("PhysicsComponent: Unsupported primitive type '{0}'", type);
}

void PhysicsComponent::render(const FrameInfo &frameInfo, VkPipelineLayout pipelineLayout)
{
#ifndef NDEBUG
    if (!CmxEditor::isActive())
    {
        return;
    }
#else
    return;
#endif
    if (getParent() == nullptr)
    {
        spdlog::critical("MeshComponent: _parent is expired");
        return;
    }
    if (_cmxShape.get() == nullptr)
    {
        return;
    }

    _cmxShape->render(frameInfo, pipelineLayout, getScene()->getAssetsManager());
}

} // namespace cmx
