#include "cmx_component.h"

// cmx
#include "cmx_actor.h"
#include "cmx_frame_info.h"

// lib
#include <spdlog/spdlog.h>

// std
#include "cxxabi.h"
#include "imgui.h"

namespace cmx
{

Component::~Component()
{
    onDetach();
}

void Component::render(FrameInfo &, VkPipelineLayout)
{
    // TODO: Debug render code here
}

void Component::editor(int i)
{
    ImGui::PushID(i);
    ImGui::Checkbox("is visible", &_isVisible);

    if (ImGui::CollapsingHeader("Transform"))
    {
        float *position[3] = {&_transform.position.x, &_transform.position.y, &_transform.position.z};
        ImGui::DragFloat3("Position", *position, 0.1f);

        float *scale[3] = {&_transform.scale.x, &_transform.scale.y, &_transform.scale.z};
        ImGui::DragFloat3("Scale", *scale, 0.1f);

        float *rotation[3] = {&_transform.rotation.x, &_transform.rotation.y, &_transform.rotation.z};
        ImGui::DragFloat3("Rotation", *rotation, 0.1f);
    }
    ImGui::PopID();
}

tinyxml2::XMLElement &Component::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentComponent)
{
    tinyxml2::XMLElement *componentElement = doc.NewElement("component");
    componentElement->SetAttribute("type", getType().c_str());
    componentElement->SetAttribute("name", name.c_str());
    componentElement->SetAttribute("visible", _isVisible);
    parentComponent->InsertEndChild(componentElement);

    tinyxml2::XMLElement *transformElement = doc.NewElement("transform");

    tinyxml2::XMLElement *positionElement = doc.NewElement("position");
    positionElement->SetAttribute("x", _transform.position.x);
    positionElement->SetAttribute("y", _transform.position.y);
    positionElement->SetAttribute("z", _transform.position.z);
    transformElement->InsertEndChild(positionElement);

    tinyxml2::XMLElement *rotationElement = doc.NewElement("rotation");
    rotationElement->SetAttribute("pitch", _transform.rotation.x);
    rotationElement->SetAttribute("yaw", _transform.rotation.y);
    rotationElement->SetAttribute("roll", _transform.rotation.z);
    transformElement->InsertEndChild(rotationElement);

    tinyxml2::XMLElement *scaleElement = doc.NewElement("scale");
    scaleElement->SetAttribute("x", _transform.scale.x);
    scaleElement->SetAttribute("y", _transform.scale.y);
    scaleElement->SetAttribute("z", _transform.scale.z);
    transformElement->InsertEndChild(scaleElement);

    componentElement->InsertEndChild(transformElement);

    return *componentElement;
}

void Component::load(tinyxml2::XMLElement *componentElement)
{
    _isVisible = componentElement->BoolAttribute("visible");

    if (tinyxml2::XMLElement *transformElement = componentElement->FirstChildElement("transform"))
    {
        if (tinyxml2::XMLElement *positionElement = transformElement->FirstChildElement("position"))
        {
            _transform.position.x = positionElement->FloatAttribute("x");
            _transform.position.y = positionElement->FloatAttribute("y");
            _transform.position.z = positionElement->FloatAttribute("z");
        }
        if (tinyxml2::XMLElement *rotationElement = transformElement->FirstChildElement("rotation"))
        {
            _transform.rotation.x = rotationElement->FloatAttribute("x");
            _transform.rotation.y = rotationElement->FloatAttribute("y");
            _transform.rotation.z = rotationElement->FloatAttribute("z");
        }
        if (tinyxml2::XMLElement *scaleElement = transformElement->FirstChildElement("scale"))
        {
            _transform.scale.x = scaleElement->FloatAttribute("x");
            _transform.scale.y = scaleElement->FloatAttribute("y");
            _transform.scale.z = scaleElement->FloatAttribute("z");
        }
    }
}

std::string Component::getType()
{
    int status;
    char *demangled = abi::__cxa_demangle(typeid(*this).name(), nullptr, nullptr, &status);

    if (status == 0)
    {
        std::string demangledString{demangled};
        free(demangled);
        return demangledString;
    }
    else
    {
        spdlog::critical("Component: Error demangling component type");
        return typeid(this).name();
    }
}

bool Component::getVisible()
{
    return _isVisible && getParent()->getVisible();
}

void Component::setParent(std::weak_ptr<Actor> actor)
{
    if (auto parent = actor.lock())
    {
        _parent = actor;
        _scene = parent->getScene();
        onAttach();
    }
}

const Transform &Component::getRelativeTransform()
{
    return _transform;
}

Transform Component::getAbsoluteTransform()
{
    if (auto parent = _parent.lock())
    {
        return parent->getAbsoluteTransform() + _transform;
    }
    return _transform;
}

} // namespace cmx
