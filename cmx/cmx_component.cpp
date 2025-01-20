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

void Component::render(const FrameInfo &, VkPipelineLayout)
{
    // TODO: Debug render code here
}

void Component::editor(int i)
{
    ImGui::PushID(i);
    ImGui::Checkbox("is visible", &_isVisible);

    _transform.editor();

    ImGui::PopID();
}

tinyxml2::XMLElement &Component::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentComponent)
{
    tinyxml2::XMLElement *componentElement = doc.NewElement("component");
    componentElement->SetAttribute("type", getType().c_str());
    componentElement->SetAttribute("name", name.c_str());
    componentElement->SetAttribute("visible", _isVisible);
    parentComponent->InsertEndChild(componentElement);

    _transform.save(doc, componentElement);

    return *componentElement;
}

void Component::load(tinyxml2::XMLElement *componentElement)
{
    _isVisible = componentElement->BoolAttribute("visible");

    if (tinyxml2::XMLElement *transformElement = componentElement->FirstChildElement("transform"))
    {
        _transform.load(transformElement);
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
    if (getParent() == nullptr)
    {
        return false;
    }

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

const Transform &Component::getRelativeTransform() const
{
    return _transform;
}

Transform Component::getAbsoluteTransform() const
{
    if (auto parent = _parent.lock())
    {
        return parent->getAbsoluteTransform() + _transform;
    }
    return _transform;
}

} // namespace cmx
