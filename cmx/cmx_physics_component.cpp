#include "cmx_physics_component.h"

// cmx
#include "cmx_actor.h"
#include "cmx_editor.h"
#include "cmx_frame_info.h"
#include "cmx_physics_manager.h"
#include "cmx_primitives.h"
#include "cmx_render_system.h"
#include "cmx_shapes.h"
#include "imgui.h"

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
    if (type.compare(PRIMITIVE_PLANE) == 0)
    {
        _cmxShape = std::shared_ptr<CmxShape>(new CmxPlane(this));
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

tinyxml2::XMLElement &PhysicsComponent::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentComponent)
{
    tinyxml2::XMLElement &componentElement = Component::save(doc, parentComponent);
    std::string name = _cmxShape->getName();
    componentElement.SetAttribute("shape", name.c_str());

    return componentElement;
}

void PhysicsComponent::load(tinyxml2::XMLElement *componentElement)
{
    Component::load(componentElement);

    setShape(componentElement->Attribute("shape"));
}

void PhysicsComponent::editor(int i)
{
    std::string selected = _cmxShape->getName();
    AssetsManager *assetsManager = getScene()->getAssetsManager();

    auto selectable = [&](const std::string &option) {
        bool isSelected = selected.compare(option) == 0;

        if (ImGui::Selectable(option.c_str(), isSelected))
        {
            selected = option;
            setShape(option);
        }

        if (isSelected)
        {
            ImGui::SetItemDefaultFocus();
        }
    };

    if (ImGui::BeginCombo("Shape##", selected.c_str()))
    {
        selectable(PRIMITIVE_SPHERE);
        selectable(PRIMITIVE_CUBE);
        selectable(PRIMITIVE_PLANE);

        ImGui::EndCombo();
    }

    Component::editor(i);
}

} // namespace cmx
