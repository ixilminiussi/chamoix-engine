#include "cmx_component.h"

// cmx
#include "cmx_actor.h"
#include "cmx_frame_info.h"

// lib
#include <spdlog/spdlog.h>

// std
#include "cxxabi.h"

namespace cmx
{

void Component::render(FrameInfo &, VkPipelineLayout)
{
    // TODO: Debug render code here
}

tinyxml2::XMLElement &Component::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentComponent)
{
    tinyxml2::XMLElement *componentElement = doc.NewElement("component");
    componentElement->SetAttribute("type", getType().c_str());
    componentElement->SetAttribute("name", name.c_str());
    parentComponent->InsertEndChild(componentElement);

    return *componentElement;
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
        return _transform + parent->getAbsoluteTransform();
    }
    return _transform;
}

} // namespace cmx
