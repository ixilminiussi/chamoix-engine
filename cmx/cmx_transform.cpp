#include "cmx_transform.h"
#include "imgui.h"
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace cmx
{

Transform Transform::ONE = Transform{};

glm::mat4 Transform::mat4() const
{
    glm::mat4 rotationMatrix = glm::toMat4(rotation); // Convert quaternion directly to mat4
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);

    return translationMatrix * rotationMatrix * scaleMatrix; // Combine transformations
}

glm::mat4 Transform::mat4_noScale() const
{
    glm::mat4 rotationMatrix = glm::toMat4(rotation); // Convert quaternion directly to mat4
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);

    return translationMatrix * rotationMatrix; // Combine transformations
}

glm::mat3 Transform::normalMatrix() const
{
    glm::mat3 rotationMatrix = glm::mat3_cast(rotation);                          // Convert quaternion to mat3
    glm::mat3 scaleMatrix = glm::mat3(glm::scale(glm::mat4(1.0f), 1.0f / scale)); // Inverse scale

    return glm::transpose(glm::inverse(rotationMatrix * scaleMatrix)); // Normal matrix calculation
}

glm::vec3 Transform::forward() const
{
    return glm::normalize(rotation * glm::vec3(0.0f, 0.0f, -1.0f)); // Default forward in OpenGL is -Z
}

glm::vec3 Transform::up() const
{
    return glm::normalize(rotation * glm::vec3(0.0f, -1.0f, 0.0f)); // Default up is +Y
}

glm::vec3 Transform::right() const
{
    return glm::normalize(rotation * glm::vec3(1.0f, 0.0f, 0.0f)); // Default right is +X
}

Transform operator+(const Transform &a, const Transform &b)
{
    Transform c{};
    glm::mat4 mat4 = a.mat4();
    c.position = mat4 * glm::vec4{b.position, 1.0f};
    c.rotation = a.rotation * b.rotation;
    c.scale = a.scale * b.scale;

    return c;
}

void Transform::editor()
{
    if (ImGui::CollapsingHeader("Transform"))
    {
        float *positionFloat[3] = {&(this->position.x), &(this->position.y), &(this->position.z)};
        ImGui::DragFloat3("Position", *positionFloat, 0.1f);

        float *scaleFloat[3] = {&(this->scale.x), &(this->scale.y), &(this->scale.z)};
        ImGui::DragFloat3("Scale", *scaleFloat, 0.1f);

        float *eulerFloat[3] = {&(_euler.x), &(_euler.y), &(_euler.z)};
        ImGui::DragFloat3("Euler", *eulerFloat, 0.1f);
        _isActive = ImGui::IsItemActive();
        ImGui::SameLine();
        ImGui::Checkbox("Additive", &_additive);
    }

    if (!_additive)
    {
        if (_isActive)
        {
            rotation = glm::quat{_euler};
        }
    }
    else
    {
        if (_wasActive && !_isActive)
        {
            rotation = glm::quat{_euler} * rotation;
            rotation = glm::normalize(rotation);
            _euler = glm::vec3{0.f, 0.f, 0.f};
        }
    }
    _wasActive = _isActive;
}

tinyxml2::XMLElement &Transform::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement)
{
    tinyxml2::XMLElement *transformElement = doc.NewElement("transform");

    tinyxml2::XMLElement *positionElement = doc.NewElement("position");
    positionElement->SetAttribute("x", position.x);
    positionElement->SetAttribute("y", position.y);
    positionElement->SetAttribute("z", position.z);
    transformElement->InsertEndChild(positionElement);

    tinyxml2::XMLElement *rotationElement = doc.NewElement("rotation");
    rotationElement->SetAttribute("pitch", rotation.x);
    rotationElement->SetAttribute("yaw", rotation.y);
    rotationElement->SetAttribute("roll", rotation.z);
    transformElement->InsertEndChild(rotationElement);

    tinyxml2::XMLElement *scaleElement = doc.NewElement("scale");
    scaleElement->SetAttribute("x", scale.x);
    scaleElement->SetAttribute("y", scale.y);
    scaleElement->SetAttribute("z", scale.z);
    transformElement->InsertEndChild(scaleElement);

    parentElement->InsertEndChild(transformElement);

    return *transformElement;
}

void Transform::load(tinyxml2::XMLElement *transformElement)
{
    if (tinyxml2::XMLElement *positionElement = transformElement->FirstChildElement("position"))
    {
        position.x = positionElement->FloatAttribute("x");
        position.y = positionElement->FloatAttribute("y");
        position.z = positionElement->FloatAttribute("z");
    }
    if (tinyxml2::XMLElement *rotationElement = transformElement->FirstChildElement("rotation"))
    {
        rotation.x = rotationElement->FloatAttribute("x");
        rotation.y = rotationElement->FloatAttribute("y");
        rotation.z = rotationElement->FloatAttribute("z");
    }
    if (tinyxml2::XMLElement *scaleElement = transformElement->FirstChildElement("scale"))
    {
        scale.x = scaleElement->FloatAttribute("x");
        scale.y = scaleElement->FloatAttribute("y");
        scale.z = scaleElement->FloatAttribute("z");
    }
}

} // namespace cmx
