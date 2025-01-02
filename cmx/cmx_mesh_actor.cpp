#include "cmx_mesh_actor.h"

// cmx
#include "cmx_mesh_component.h"
#include "imgui.h"

// lib
#include <glm/ext/scalar_constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace cmx
{

void MeshActor::onBegin()
{
    Actor::onBegin();

    _meshComponent = std::make_shared<MeshComponent>();
    attachComponent(_meshComponent);
}

void MeshActor::update(float dt)
{
}

void MeshActor::editor()
{
    ImGui::Checkbox("is visible", &_isVisible);

    Actor::editor();
}

tinyxml2::XMLElement &MeshActor::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement)
{
    tinyxml2::XMLElement &actorElement = Actor::save(doc, parentElement);
    actorElement.SetAttribute("visible", _isVisible);

    return actorElement;
}

void MeshActor::load(tinyxml2::XMLElement *actorElement)
{
    _isVisible = actorElement->BoolAttribute("visible");

    Actor::load(actorElement);
}

} // namespace cmx
