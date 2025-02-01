#include "cmx_physics_component.h"

// cmx
#include "cmx/cmx_physics.h"
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

void PhysicsComponent::setPhysicsMode(PhysicsMode newMode)
{
    _physicsMode = newMode;
    getScene()->getPhysicsManager()->add(shared_from_this());
}

void PhysicsComponent::setShape(const std::string &type)
{
    if (type.compare(PRIMITIVE_SPHERE) == 0)
    {
        _shape = std::shared_ptr<Shape>(new Sphere(this));
        _shape->setMask(_mask);
        return;
    }
    if (type.compare(PRIMITIVE_CUBE) == 0)
    {
        _shape = std::shared_ptr<Shape>(new Cuboid(this));
        _shape->setMask(_mask);
        return;
    }
    if (type.compare(PRIMITIVE_PLANE) == 0)
    {
        _shape = std::shared_ptr<Shape>(new Plane(this));
        _shape->setMask(_mask);
        return;
    }

    spdlog::warn("PhysicsComponent: Unsupported primitive type '{0}'", type);
}

void PhysicsComponent::setMask(uint8_t mask)
{
    _mask = mask;
    _shape->setMask(mask);
}

void PhysicsComponent::render(const FrameInfo &frameInfo, vk::PipelineLayout pipelineLayout)
{
#ifndef NDEBUG
    if (!Editor::isActive())
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
    if (_shape.get() == nullptr)
    {
        return;
    }

    _shape->render(frameInfo, pipelineLayout, getScene()->getAssetsManager());
}

tinyxml2::XMLElement &PhysicsComponent::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentComponent)
{
    tinyxml2::XMLElement &componentElement = Component::save(doc, parentComponent);
    std::string name = _shape->getName();
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
    {
        std::string selected = (_shape.get() != nullptr) ? _shape->getName() : "";

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
    }

    {
        std::string selected = physicsModeToString(_physicsMode);

        auto selectable = [&](PhysicsMode physicsMode) {
            std::string option = physicsModeToString(physicsMode);
            bool isSelected = selected.compare(option) == 0;

            if (ImGui::Selectable(option.c_str(), isSelected))
            {
                selected = option;
                setPhysicsMode(physicsMode);
            }

            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        };

        if (ImGui::BeginCombo("PhysicsMode##", selected.c_str()))
        {
            selectable(PhysicsMode::STATIC);
            selectable(PhysicsMode::DYNAMIC);
            selectable(PhysicsMode::RIGID);

            ImGui::EndCombo();
        }
    }

    Component::editor(i);
}

} // namespace cmx
