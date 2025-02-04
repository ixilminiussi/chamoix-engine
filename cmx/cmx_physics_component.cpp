#include "cmx_physics_component.h"

// cmx
#include "cmx_actor.h"
#include "cmx_editor.h"
#include "cmx_frame_info.h"
#include "cmx_physics.h"
#include "cmx_physics_manager.h"
#include "cmx_primitives.h"
#include "cmx_render_system.h"
#include "cmx_shapes.h"
#include "imgui.h"
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/quaternion.hpp>

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

glm::vec3 PhysicsComponent::getCenterOfMassLocalSpace() const
{
    return _shape->getCenterOfMass();
}

glm::vec3 PhysicsComponent::getCenterOfMassWorldSpace() const
{
    return glm::vec3(getWorldSpaceTransform().mat4() * glm::vec4(_shape->getCenterOfMass(), 1.0f));
}

glm::mat3 PhysicsComponent::getInverseInertiaTensorLocalSpace() const
{
    return glm::inverse(_shape->getInertiaTensor()) * _inverseMass;
}

glm::mat3 PhysicsComponent::getInverseInertiaTensorWorldSpace() const
{
    glm::mat3 inverseInertiaTensor = glm::inverse(_shape->getInertiaTensor()) * _inverseMass;
    glm::mat3 orient = glm::mat3_cast(getWorldSpaceTransform().rotation);

    return orient * inverseInertiaTensor * glm::transpose(orient);
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
    componentElement.SetAttribute("physicsMode", physicsModeToString(_physicsMode));

    if (_physicsMode == PhysicsMode::RIGID)
    {
        componentElement.SetAttribute("gravityX", _gravity.x);
        componentElement.SetAttribute("gravityY", _gravity.y);
        componentElement.SetAttribute("gravityZ", _gravity.z);
        componentElement.SetAttribute("inverseMass", _inverseMass);
    }

    componentElement.SetAttribute("bounciness", _bounciness);

    return componentElement;
}

void PhysicsComponent::load(tinyxml2::XMLElement *componentElement)
{
    Component::load(componentElement);

    try
    {
        setShape(componentElement->Attribute("shape"));
        const char *physicsModeStr = componentElement->Attribute("physicsMode");
        setPhysicsMode(physicsModeFromString(physicsModeStr));

        if (_physicsMode == PhysicsMode::RIGID)
        {
            _gravity.x = componentElement->FloatAttribute("gravityX");
            _gravity.y = componentElement->FloatAttribute("gravityY");
            _gravity.z = componentElement->FloatAttribute("gravityZ");
            _inverseMass = componentElement->FloatAttribute("inverseMass");
            if (_inverseMass <= glm::epsilon<float>())
                _inverseMass = 0.f;
        }

        _bounciness = componentElement->FloatAttribute("bounciness");
    }
    catch (...)
    {
    }
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

    ImGui::DragFloat("bounciness", &_bounciness, 0.5f, 0.f, 1.0f);

    if (_physicsMode == PhysicsMode::RIGID)
    {
        ImGui::DragFloat("inverse mass", &_inverseMass, .1f, 0.f, 10.f);
        ImGui::DragFloat3("gravity", (float *)&_gravity, 0.f, 100.f);
    }

    Component::editor(i);
}

void PhysicsComponent::applyCollision(float dt, const HitInfo &hitInfo, const PhysicsComponent &other)
{
    if (getParent() == nullptr)
        return;

    if (_inverseMass == 0.f)
        return;

    const Transform transform = getParent()->getWorldSpaceTransform();

    const float otherInvMass = other.isRigid() ? other._inverseMass : 0.f;

    const float t = _inverseMass / (otherInvMass + _inverseMass);
    const glm::vec3 d = hitInfo.normal * hitInfo.depth;

    getParent()->setPosition(transform.position - (t * d));

    // Collision impulse
    const glm::vec3 &relVelocity = _linearVelocity - other._linearVelocity;
    const float impulse = (-(1.0f + _bounciness * other._bounciness) * glm::dot(hitInfo.normal, relVelocity)) /
                          (_inverseMass + otherInvMass);

    applyImpulseLinear(hitInfo.normal * impulse);

    _shape->reassess();
}

void PhysicsComponent::applyImpulseLinear(const glm::vec3 &impulse)
{
    _linearVelocity += impulse * _inverseMass;
}

void PhysicsComponent::applyGravity(float dt)
{
    if (_inverseMass > 0.f)
    {
        float mass = 1.f / _inverseMass;
        glm::vec3 impulseGravity = _gravity * mass * dt;

        applyImpulseLinear(impulseGravity);
    }
}

void PhysicsComponent::applyImpulse(const glm::vec3 &impulseOrigin, const glm::vec3 &impulse)
{
    if (_inverseMass == 0.0f)
        return;

    applyImpulseLinear(impulse);

    glm::vec3 r = impulseOrigin - getCenterOfMassWorldSpace();
    applyImpulseAngular(glm::cross(r, impulse));
}

void PhysicsComponent::applyImpulseAngular(const glm::vec3 &impulse)
{
    if (_inverseMass == 0.0f)
        return;

    _angularVelocity += getInverseInertiaTensorWorldSpace() * impulse;

    const float maxAngularSpeed = 30.0f;
    if (_angularVelocity.length() > maxAngularSpeed)
    {
        _angularVelocity = glm::normalize(_angularVelocity) * maxAngularSpeed;
    }
}

void PhysicsComponent::applyVelocity(float dt)
{
    if (getParent())
        getParent()->setPosition(getParent()->getWorldSpaceTransform().position += _linearVelocity * dt);
}

} // namespace cmx
