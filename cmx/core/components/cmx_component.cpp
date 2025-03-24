#include "cmx_component.h"

// cmx
#include "cmx_actor.h"
#include "cmx_frame_info.h"
#include "cmx_register.h"

// lib
#include <spdlog/spdlog.h>

// std
#ifndef _WIN32
#include <cxxabi.h>
#else
#include <regex>
#endif

#include <imgui.h>

REGISTER_COMPONENT(cmx::Component)

namespace cmx
{

Component::~Component()
{
    onDetach();
}

void Component::render(const FrameInfo &, vk::PipelineLayout)
{
    // TODO: Debug render code here
}

void Component::editor(int i)
{
    ImGui::PushID(i);

    _transform.editor();

    ImGui::PopID();
}

tinyxml2::XMLElement &Component::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const
{
    tinyxml2::XMLElement *componentElement = doc.NewElement("component");
    componentElement->SetAttribute("type", getType().c_str());
    componentElement->SetAttribute("name", name.c_str());
    parentElement->InsertEndChild(componentElement);

    _transform.save(doc, componentElement);

    return *componentElement;
}

void Component::load(tinyxml2::XMLElement *componentElement)
{
    if (tinyxml2::XMLElement *transformElement = componentElement->FirstChildElement("transform"))
    {
        _transform.load(transformElement);
    }
}

#ifdef _WIN32
std::string Component::getType() const
{
    return std::regex_replace(typeid(*this).name(), std::regex(R"(^(class |struct ))"), "");
}
#else
std::string Component::getType() const
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
#endif

void Component::setParent(Actor *actor)
{
    _parent = actor;

    if (_parent != nullptr)
    {
        _scene = _parent->getScene();
        onAttach();
    }
}

const Transform &Component::getLocalSpaceTransform() const
{
    return _transform;
}

Transform Component::getWorldSpaceTransform(int depth) const
{
    if (_parent == nullptr || depth == 0)
    {
        return getLocalSpaceTransform();
    }

    if (depth == -1)
    {
        return getLocalSpaceTransform() + _parent->getWorldSpaceTransform();
    }

    return getLocalSpaceTransform() + _parent->getWorldSpaceTransform(depth - 1);
}

} // namespace cmx
