#include "cmx_actor.h"
#include "imgui.h"

// lib
#include <spdlog/spdlog.h>

// std
#include <memory>
#include <spdlog/common.h>

namespace cmx
{

Actor::Actor(World *world, uint32_t id, const std::string &name, const Transform &transform)
    : world{world}, id{id}, name{name}, transform{transform}
{
}

void Actor::despawn()
{
    getWorld()->removeActor(this);
    // TODO: remove components as well
}

void Actor::move(World *world)
{
    getWorld()->addActor(std::shared_ptr<Actor>(this));
    getWorld()->removeActor(this);
    // TODO: move components as well
}

void Actor::attachComponent(std::shared_ptr<Component> component)
{
    component->setParent(this);

    components.push_back(component);
    getWorld()->addComponent(component);
}

Transform Actor::getAbsoluteTransform()
{
    if (positioning == Positioning::RELATIVE)
    {
        if (auto parentActor = parent.lock())
        {
            return transform + parentActor->getAbsoluteTransform();
        }
    }

    return transform;
}

void Actor::renderSettings()
{
    ImGui::SeparatorText("Transform");
    float *position[3] = {&transform.position.x, &transform.position.y, &transform.position.z};
    ImGui::DragFloat3("Position", *position, 0.1f);
    float *scale[3] = {&transform.scale.x, &transform.scale.y, &transform.scale.z};
    ImGui::DragFloat3("Scale", *scale, 0.1f);
    float *rotation[3] = {&transform.rotation.x, &transform.rotation.y, &transform.rotation.z};
    ImGui::DragFloat3("Rotation", *rotation, 0.1f);

    if (components.size() > 0)
    {
        ImGui::SeparatorText("Components");
        for (auto component : components)
        {
            if (ImGui::TreeNode(component->name))
            {
                component->renderSettings();
                ImGui::TreePop();
            }
        }
    }
}

} // namespace cmx
