#include "cmx_physics_body.h"

// cmx
#include "cmx_actor.h"
#include "cmx_editor.h"
#include "cmx_game.h"
#include "cmx_math.h"
#include "cmx_physics_manager.h"
#include "cmx_primitives.h"
#include "cmx_shapes.h"

namespace cmx
{

PhysicsBody::PhysicsBody(class Actor **parentP) : _parentP{parentP}
{
}

void PhysicsBody::setPhysicsMode(PhysicsMode newMode)
{
    _physicsMode = newMode;

    if (getParentActor() == nullptr)
        return;

    getParentActor()->getScene()->getPhysicsManager()->add(this);
}

void PhysicsBody::setShape(const std::string &type)
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

    spdlog::warn("PhysicsBody: Unsupported primitive type '{0}'", type);
}

void PhysicsBody::setMask(uint8_t mask)
{
    _mask = mask;
    _shape->setMask(mask);
}

glm::vec3 PhysicsBody::getCenterOfMassLocalSpace() const
{
    return _shape->getCenterOfMass();
}

glm::vec3 PhysicsBody::getCenterOfMassWorldSpace() const
{
    return glm::vec3(getWorldSpaceTransform().mat4() * glm::vec4(_shape->getCenterOfMass(), 1.0f));
}

glm::mat3 PhysicsBody::getInverseInertiaTensorLocalSpace() const
{
    return glm::inverse(_shape->getInertiaTensor()) * _inverseMass;
}

glm::mat3 PhysicsBody::getInverseInertiaTensorWorldSpace() const
{
    glm::mat3 orient = glm::mat3_cast(getWorldSpaceTransform().rotation);

    return orient * getInverseInertiaTensorLocalSpace() * glm::transpose(orient);
}

// void PhysicsBody::render(const FrameInfo &frameInfo, vk::PipelineLayout pipelineLayout)
// {
// #ifndef NDEBUG
//     if (!Editor::isActive())
//     {
//         return;
//     }
// #else
//     return;
// #endif
//     if (getParentActor() == nullptr)
//     {
//         spdlog::critical("MeshComponent: _parent is expired");
//         return;
//     }
//     if (_shape.get() == nullptr)
//     {
//         return;
//     }
//
//     _shape->render(frameInfo, pipelineLayout, getParentActor()->getScene()->getAssetsManager());
// }

void PhysicsBody::applyCollision(float dt, const HitInfo &hitInfo, const PhysicsBody &other)
{
    if (getParentActor() == nullptr)
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

        const glm::vec3 inertia =
            glm::cross((inverseWorldInertia * glm::cross(relHitPoint, normalizedVelTangent)), relHitPoint);
        const glm::vec3 otherInertia = glm::cross(
            (otherInverseWorldInertia * glm::cross(otherRelHitPoint, normalizedVelTangent)), otherRelHitPoint);
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
    }

    // -- If object are interpenetrating,
    // -- use this to set them on contact
    getParentActor()->setPosition(getWorldSpaceTransform().position +
                                  (-hitInfo.normal * hitInfo.depth) *
                                      (_inverseMass / (_inverseMass + other._inverseMass)));

    // Resistance
    _linearVelocity = cmx::lerp(_linearVelocity, glm::vec3{0.f}, _airResistance * dt);
    _angularVelocity = cmx::lerp(_angularVelocity, glm::vec3{0.f}, _airResistance * dt);

    _shape->reassess();
}

void PhysicsBody::applyGravity(float dt)
{
    if (_inverseMass > 0.f)
    {
        float mass = 1.f / _inverseMass;
        glm::vec3 impulseGravity = _gravity * mass * dt;

        applyImpulseLinear(impulseGravity);
    }
}

void PhysicsBody::applyImpulse(const glm::vec3 &impulseOrigin, const glm::vec3 &impulse)
{
    if (_inverseMass == 0.0f)
        return;

    applyImpulseLinear(impulse);

    const glm::vec3 r = impulseOrigin - getCenterOfMassWorldSpace();
    applyImpulseAngular(glm::cross(r, impulse));
}

void PhysicsBody::applyImpulseLinear(const glm::vec3 &impulse)
{
    _linearVelocity += impulse * _inverseMass;
}

void PhysicsBody::applyImpulseAngular(const glm::vec3 &impulse)
{
    _angularVelocity += getInverseInertiaTensorWorldSpace() * impulse;

    const float maxAngularSpeed = 30.f;
    if (glm::length(_angularVelocity) > maxAngularSpeed)
    {
        _angularVelocity = glm::normalize(_angularVelocity) * maxAngularSpeed;
    }
}

void PhysicsBody::applyVelocity(float dt)
{
    if (!getParentActor())
        return;

    Transform transform = getParentActor()->getWorldSpaceTransform();
    getParentActor()->setPosition(transform.position + _linearVelocity * dt);

    transform = getParentActor()->getWorldSpaceTransform();
    const glm::vec3 centerOfMass = getCenterOfMassWorldSpace();
    const glm::vec3 CMToPosition = transform.position - centerOfMass;

    const glm::mat3 orientationMatrix = glm::mat3_cast(transform.rotation);
    const glm::mat3 inertiaTensor =
        orientationMatrix * getInverseInertiaTensorLocalSpace() * glm::transpose(orientationMatrix);

    if (glm::length(_angularVelocity) <= glm::epsilon<float>())
        return;

    const glm::vec3 alpha =
        glm::inverse(inertiaTensor) * (glm::cross(_angularVelocity, inertiaTensor * _angularVelocity));

    _angularVelocity += alpha * dt;

    const glm::vec3 dAngle = _angularVelocity * dt;
    const glm::quat dq = glm::quat_cast(glm::rotate<float>(glm::mat4(1.0f), glm::length(dAngle), dAngle));

    getParentActor()->setRotation(dq * transform.rotation);
    getParentActor()->setPosition(centerOfMass + glm::vec3(glm::mat4_cast(dq) * glm::vec4(CMToPosition, 1.0f)));
}

tinyxml2::XMLElement &PhysicsBody::save(tinyxml2::XMLElement &componentElement) const
{
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

void PhysicsBody::load(tinyxml2::XMLElement *componentElement)
{
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

void PhysicsBody::editor(int i)
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
}

void PhysicsBody::setMass(float mass)
{
    _inverseMass = 1.f / glm::max(glm::epsilon<float>(), mass);
}

void PhysicsBody::setInverseMass(float inverseMass)
{
    _inverseMass = glm::max(0.f, inverseMass);
}

} // namespace cmx
