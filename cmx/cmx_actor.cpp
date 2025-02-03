#include "cmx_actor.h"

// cmx
#include "cmx_component.h"
#include "cmx_register.h"

// lib
#include <IconsMaterialSymbols.h>
#include <imgui.h>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <tinyxml2.h>

// std
#include <cxxabi.h>
#include <memory>
#include <stdexcept>

namespace cmx
{

Actor::Actor(Scene *scene, uint32_t id, const std::string &name, const Transform &transform)
    : Transformable{transform}, _scene{scene}, _id{id}, name{name}
{
}

Actor::~Actor()
{
    for (auto &pair : _components)
    {
        pair.second->setParent(nullptr);
    }
}

void Actor::despawn()
{
    _state = State::DEAD;

    for (auto &pair : _components)
    {
        pair.second->setParent(nullptr);
    }
}

std::shared_ptr<Component> Actor::attachComponent(std::shared_ptr<Component> component, std::string componentName)
{
    componentName = (componentName.compare("") == 0) ? component->getType() : componentName;
    auto it = _components.find(componentName);
    if (it != _components.end())
    {
        spdlog::warn("Actor {0}: component of same name <{1}> already bound to actor", name, componentName);
        return it->second;
    }

    Actor *actor = component->getParent();
    if (actor != nullptr)
    {
        actor->detachComponent(componentName);
    }
    component->name = componentName;
    component->setParent(this);

    _components[component->name] = component;
    getScene()->addComponent(component);

    return component;
}

void Actor::detachComponent(const std::string &componentName)
{
    auto it = _components.find(componentName);
    if (it == _components.end())
    {
        spdlog::warn("Actor {0}: Attempt to remove unheld component <{1}>'", name, componentName);
        return;
    }

    it->second->setParent(nullptr);
    _components.erase(componentName);

    spdlog::info("Actor {0}: Removed component <{1}>", name, componentName);
}

std::weak_ptr<Component> Actor::getComponentByName(const std::string &name)
{
    std::weak_ptr<Component> component;

    auto it = _components.find(name);
    if (it == _components.end())
    {
        spdlog::warn("Actor {0}: Attempt to get component from invalid id: {1}'", name, _id);
        return component;
    }

    return it->second;
}

Transform Actor::getWorldSpaceTransform() const
{
    if (positioning == Positioning::RELATIVE)
    {
        if (auto parentActor = _parent.lock())
        {
            return _transform + parentActor->getWorldSpaceTransform();
        }
    }

    return _transform;
}

void Actor::editor()
{
    ImGui::Checkbox("is visible", &_isVisible);

    if (auto camera = getScene()->getCamera().lock())
    {
        Transformable::editor(camera.get());
    }

    int i = 0;
    if (_components.size() > 0)
    {
        std::string markedForDeletion{};
        bool deleteMarked{false};

        ImGui::SeparatorText("Components");
        std::string label;

        for (auto component : _components)
        {
            if (ImGui::TreeNode(component.first.c_str()))
            {
                component.second->editor(i++);
                ;
                if (ImGui::Button(ICON_MS_DELETE))
                {
                    markedForDeletion = component.first;
                    deleteMarked = true;
                }
                ImGui::TreePop();
            }
        }

        if (deleteMarked)
        {
            detachComponent(markedForDeletion);
        }
    }

    // create new actor
    Register *cmxRegister = Register::getInstance();
    static const char *selected = cmxRegister->getComponentRegister().begin()->first.c_str();

    ImGui::SeparatorText("New Component");
    ImGui::SetNextItemWidth(203);
    if (ImGui::BeginCombo("##unique2", selected))
    {
        for (const auto &pair : cmxRegister->getComponentRegister())
        {
            bool isSelected = (strcmp(selected, pair.first.c_str()) == 0);

            if (ImGui::Selectable(pair.first.c_str(), isSelected))
            {
                selected = pair.first.c_str();
            }

            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

    static char buffer[100];
    ImGui::SetNextItemWidth(170);
    ImGui::InputText("##unique4", buffer, 100);
    ImGui::SameLine();
    if (ImGui::Button(ICON_MS_ADD "##unique5"))
    {
        cmxRegister->attachComponent(selected, this, buffer);
    }
}

tinyxml2::XMLElement &Actor::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement)
{
    tinyxml2::XMLElement *actorElement = doc.NewElement("actor");
    actorElement->SetAttribute("type", getType().c_str());
    actorElement->SetAttribute("name", name.c_str());
    actorElement->SetAttribute("id", _id);
    actorElement->SetAttribute("visible", _isVisible);

    _transform.save(doc, actorElement);

    for (auto componentPair : _components)
    {
        componentPair.second->save(doc, actorElement);
    }

    parentElement->InsertEndChild(actorElement);

    return *actorElement;
}

void Actor::load(tinyxml2::XMLElement *actorElement)
{
    _isVisible = actorElement->BoolAttribute("visible");

    if (tinyxml2::XMLElement *transformElement = actorElement->FirstChildElement("transform"))
    {
        _transform.load(transformElement);
    }

    Register *cmxRegister = Register::getInstance();
    tinyxml2::XMLElement *componentElement = actorElement->FirstChildElement("component");
    while (componentElement)
    {
        cmxRegister->attachComponent(componentElement->Attribute("type"), this, componentElement->Attribute("name"))
            ->load(componentElement);

        // TODO: Implement this shit
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
        spdlog::critical("Actor {0}: Error demangling component type", name);
        return typeid(this).name();
    }
}

} // namespace cmx
