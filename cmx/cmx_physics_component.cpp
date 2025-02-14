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

// lib
#include "imgui.h"
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/geometric.hpp>
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
    glm::mat3 orient = glm::mat3_cast(getWorldSpaceTransform().rotation);

    return orient * getInverseInertiaTensorLocalSpace() * glm::transpose(orient);
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

void PhysicsComponent::applyCollision(float dt, const HitInfo &hitInfo, const PhysicsComponent &other)
{
    if (getParent() == nullptr)
        return;

    if (_inverseMass == 0.f)
        return;

    const float bounciness = _bounciness * other._bounciness;

    const glm::mat3 inverseWorldInertia = getInverseInertiaTensorWorldSpace();
    const glm::mat3 otherInverseWorldInertia = other.getInverseInertiaTensorWorldSpace();

    const glm::vec3 relHitPoint = hitInfo.point - getCenterOfMassWorldSpace();
    const glm::vec3 otherRelHitPoint = hitInfo.getFlipped().point - other.getCenterOfMassWorldSpace();

    const glm::vec3 angularJ = glm::cross(inverseWorldInertia * glm::cross(relHitPoint, -hitInfo.normal), relHitPoint);
    const glm::vec3 otherAngularJ =
        glm::cross(otherInverseWorldInertia * glm::cross(otherRelHitPoint, hitInfo.normal), otherRelHitPoint);
    const float angularFactor = glm::dot((angularJ + otherAngularJ), -hitInfo.normal);

    // Get world space velocity of the motion and rotation
    const glm::vec3 combinedVel = _linearVelocity + glm::cross(_angularVelocity, relHitPoint);
    const glm::vec3 otherCombinedVel = other._linearVelocity + glm::cross(other._angularVelocity, otherRelHitPoint);

    // Collision impulse
    const glm::vec3 relVel = combinedVel - otherCombinedVel;

    // -- Sign is changed here
    const float impulseValueJ =
        (1.0f + bounciness) * glm::dot(relVel, -hitInfo.normal) / (_inverseMass + other._inverseMass + angularFactor);

    const glm::vec3 impulse = hitInfo.normal * impulseValueJ;
    applyImpulse(hitInfo.point, impulse);

    // Friction-caused impulse
    const float staticFriction = _friction * other._friction;
    const float dynamicFriction = staticFriction * 0.7f;

    // -- Find the normal direction of the velocity
    // -- with respect to the normal of the collision
    const glm::vec3 velNormal = -hitInfo.normal * glm::dot(hitInfo.normal, relVel);

    // -- Find the tengent direction of the velocity
    // -- with respect to the normal of the collision
    const glm::vec3 velTangent = relVel - velNormal;

    // -- Get the tengential velocities relative to the other body
    glm::vec3 normalizedVelTangent = velTangent;
    if (glm::length(velTangent) > glm::epsilon<float>())
    {
        normalizedVelTangent = glm::normalize(normalizedVelTangent);
    }

    const glm::vec3 inertia =
        glm::cross((inverseWorldInertia * glm::cross(relHitPoint, normalizedVelTangent)), relHitPoint);
    const glm::vec3 otherInertia =
        glm::cross((otherInverseWorldInertia * glm::cross(otherRelHitPoint, normalizedVelTangent)), otherRelHitPoint);
    const float inverseInertia = glm::dot(inertia + otherInertia, normalizedVelTangent);

    // -- Tangential impulse for friction
    const float reducedMass = 1.0f / (_inverseMass + other._inverseMass + inverseInertia);

    const float maxStaticFrictionImpulse = impulseValueJ * staticFriction;
    const glm::vec3 desiredFrictionImpulse = -velTangent * reducedMass;

    glm::vec3 impulseFriction;
    if (glm::length(desiredFrictionImpulse) <= maxStaticFrictionImpulse)
    {
        impulseFriction = desiredFrictionImpulse;
    }
    else
    {
        impulseFriction = glm::normalize(velTangent) * maxStaticFrictionImpulse * dynamicFriction;
    }

    // -- Apply kinetic friction
    applyImpulse(hitInfo.point, impulseFriction);

    // -- If object are interpenetrating,
    // -- use this to set them on contact
    getParent()->setPosition(getWorldSpaceTransform().position +
                             (-hitInfo.normal * hitInfo.depth) * (_inverseMass / (_inverseMass + other._inverseMass)));

    _shape->reassess();
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

    const glm::vec3 r = impulseOrigin - getCenterOfMassWorldSpace();
    applyImpulseAngular(glm::cross(r, impulse));
}

void PhysicsComponent::applyImpulseLinear(const glm::vec3 &impulse)
{
    _linearVelocity += impulse * _inverseMass;
}

void PhysicsComponent::applyImpulseAngular(const glm::vec3 &impulse)
{
    _angularVelocity += getInverseInertiaTensorWorldSpace() * impulse;

    const float maxAngularSpeed = 30.f;
    if (glm::length(_angularVelocity) > maxAngularSpeed)
    {
        _angularVelocity = glm::normalize(_angularVelocity) * maxAngularSpeed;
    }
}

void PhysicsComponent::applyVelocity(float dt)
{
    if (!getParent())
        return;

    Transform transform = getParent()->getWorldSpaceTransform();
    getParent()->setPosition(transform.position + _linearVelocity * dt);

    transform = getParent()->getWorldSpaceTransform();
    const glm::vec3 centerOfMass = getCenterOfMassWorldSpace();
    const glm::vec3 CMToPosition = transform.position - centerOfMass;

    const glm::mat3 orientationMatrix = glm::mat3_cast(transform.rotation);
    const glm::mat3 inertiaTensor =
        orientationMatrix * getInverseInertiaTensorLocalSpace() * glm::transpose(orientationMatrix);

    if (_angularVelocity == glm::vec3{0.f})
        return;

    const glm::vec3 alpha =
        glm::inverse(inertiaTensor) * (glm::cross(_angularVelocity, inertiaTensor * _angularVelocity));

    _angularVelocity += alpha * dt;

    const glm::vec3 dAngle = _angularVelocity * dt;
    const glm::quat dq = glm::quat_cast(glm::rotate<float>(glm::mat4(1.0f), glm::length(dAngle), dAngle));

    getParent()->setRotation(dq * transform.rotation);
    getParent()->setPosition(centerOfMass + glm::vec3(glm::mat4_cast(dq) * glm::vec4(CMToPosition, 1.0f)));
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
    componentElement.SetAttribute("friction", _friction);

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
        _friction = componentElement->FloatAttribute("friction");
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

    ImGui::DragFloat("bounciness", &_bounciness, 0.05f, 0.f, 1.0f);
    ImGui::DragFloat("friction", &_friction, 0.05f, 0.f, 1.0f);

    if (_physicsMode == PhysicsMode::RIGID)
    {
        ImGui::DragFloat("inverse mass", &_inverseMass, .1f, 0.f, 10.f);
        ImGui::DragFloat3("gravity", (float *)&_gravity, 0.f, 100.f);
    }

    Component::editor(i);
}

} // namespace cmx
