#include "cmx_actor.h"

// cmx
#include "cmx_component.h"

// lib
#include "imgui.h"
#include "tinyxml2.h"
#include <spdlog/spdlog.h>

// std
#include "cxxabi.h"
#include <memory>
#include <spdlog/common.h>

namespace cmx
{

Actor::Actor(Scene *scene, uint32_t id, const std::string &name, const Transform &transform)
    : _scene{scene}, _id{id}, name{name}, transform{transform}
{
}

Actor::~Actor()
{
}

void Actor::despawn()
{
    getScene()->removeActor(this);
    // TODO: remove components as well
}

void Actor::move(Scene *scene)
{
    getScene()->addActor(std::shared_ptr<Actor>(this));
    getScene()->removeActor(this);
    // TODO: move components as well
}

void Actor::attachComponent(std::shared_ptr<Component> component, std::string componentName)
{
    componentName = (componentName.compare("") == 0) ? component->getType() : componentName;
    try
    {
        auto c = _components.at(componentName);
    }
    catch (const std::out_of_range &e)
    {
        if (component->getParent() != nullptr)
        {
            component->getParent()->detachComponent(component->name);
        }
        component->name = componentName;
        component->setParent(this);

        _components[component->name] = component;
        getScene()->addComponent(component);

        return;
    }
    spdlog::warn("Actor '{0}': component of same name '{1}' already bound to actor", name, componentName);
    return;
}

void Actor::detachComponent(const std::string &componentName)
{
    try
    {
        _components.at(componentName) = nullptr;
    }
    catch (const std::out_of_range &e)
    {
        spdlog::warn("Actor '{0}': Attempt to remove unheld component '{1}''", name, componentName);
    }

    _components.erase(componentName);

    spdlog::info("Actor '{0}': Removed component '{1}'", name, componentName);
}

std::weak_ptr<Component> Actor::getComponentByName(const std::string &name)
{
    std::weak_ptr<Component> component;
    try
    {
        component = _components.at(name);
    }
    catch (const std::out_of_range &e)
    {
        spdlog::warn("Scene '{0}': Attempt to get component from invalid id: '{1}''", name, _id);
    }

    return component;
}

Transform Actor::getAbsoluteTransform()
{
    if (positioning == Positioning::RELATIVE)
    {
        if (auto parentActor = _parent.lock())
        {
            return transform + parentActor->getAbsoluteTransform();
        }
    }

    return transform;
}

void Actor::renderSettings()
{
    if (ImGui::CollapsingHeader("Transform"))
    {
        float *position[3] = {&transform.position.x, &transform.position.y, &transform.position.z};
        ImGui::DragFloat3("Position", *position, 0.1f);

        float *scale[3] = {&transform.scale.x, &transform.scale.y, &transform.scale.z};
        ImGui::DragFloat3("Scale", *scale, 0.1f);

        float *rotation[3] = {&transform.rotation.x, &transform.rotation.y, &transform.rotation.z};
        ImGui::DragFloat3("Rotation", *rotation, 0.1f);
    }

    if (_components.size() > 0)
    {
        ImGui::SeparatorText("Components");
        int i = 0;
        std::string label;

        for (auto component : _components)
        {
            label = fmt::format("{}##{}", component.first.c_str(), i++);
            if (ImGui::CollapsingHeader(label.c_str()))
            {
                component.second->renderSettings(i);
            }
        }
    }
}

tinyxml2::XMLElement &Actor::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement)
{
    tinyxml2::XMLElement *actorElement = doc.NewElement("actor");
    actorElement->SetAttribute("type", getType().c_str());
    actorElement->SetAttribute("name", name.c_str());
    actorElement->SetAttribute("id", _id);

    tinyxml2::XMLElement *transformElement = doc.NewElement("transform");

    tinyxml2::XMLElement *positionElement = doc.NewElement("position");
    positionElement->SetAttribute("x", transform.position.x);
    positionElement->SetAttribute("y", transform.position.y);
    positionElement->SetAttribute("z", transform.position.z);
    transformElement->InsertEndChild(positionElement);

    tinyxml2::XMLElement *rotationElement = doc.NewElement("rotation");
    rotationElement->SetAttribute("pitch", transform.rotation.x);
    rotationElement->SetAttribute("yaw", transform.rotation.y);
    rotationElement->SetAttribute("roll", transform.rotation.z);
    transformElement->InsertEndChild(rotationElement);

    tinyxml2::XMLElement *scaleElement = doc.NewElement("scale");
    scaleElement->SetAttribute("x", transform.scale.x);
    scaleElement->SetAttribute("y", transform.scale.y);
    scaleElement->SetAttribute("z", transform.scale.z);
    transformElement->InsertEndChild(scaleElement);

    actorElement->InsertEndChild(transformElement);

    for (auto componentPair : _components)
    {
        componentPair.second->save(doc, actorElement);
    }

    parentElement->InsertEndChild(actorElement);

    return *actorElement;
}

void Actor::load(tinyxml2::XMLElement *actorElement)
{
    if (tinyxml2::XMLElement *transformElement = actorElement->FirstChildElement("transform"))
    {
        if (tinyxml2::XMLElement *positionElement = transformElement->FirstChildElement("position"))
        {
            transform.position.x = positionElement->FloatAttribute("x");
            transform.position.y = positionElement->FloatAttribute("y");
            transform.position.z = positionElement->FloatAttribute("z");
        }
        if (tinyxml2::XMLElement *rotationElement = transformElement->FirstChildElement("rotation"))
        {
            transform.rotation.x = rotationElement->FloatAttribute("x");
            transform.rotation.y = rotationElement->FloatAttribute("y");
            transform.rotation.z = rotationElement->FloatAttribute("z");
        }
        if (tinyxml2::XMLElement *scaleElement = transformElement->FirstChildElement("scale"))
        {
            transform.scale.x = scaleElement->FloatAttribute("x");
            transform.scale.y = scaleElement->FloatAttribute("y");
            transform.scale.z = scaleElement->FloatAttribute("z");
        }
    }

    tinyxml2::XMLElement *componentElement = actorElement->FirstChildElement("component");
    while (componentElement)
    {
        componentElement = componentElement->NextSiblingElement("component");
    }
}

std::string Actor::getType()
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

} // namespace cmx
