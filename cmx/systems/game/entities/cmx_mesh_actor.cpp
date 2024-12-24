#include "cmx_mesh_actor.h"

// cmx
#include "cmx_game.h"
#include "cmx_input_manager.h"
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
    auto meshComponent = std::make_shared<cmx::MeshComponent>();
    attachComponent(meshComponent);

    meshComponent->setModel("bunny");
}

void MeshActor::update(float dt)
{
}

void MeshActor::renderSettings()
{
    Actor::renderSettings();
}

tinyxml2::XMLElement &MeshActor::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement)
{
    tinyxml2::XMLElement &actorElement = Actor::save(doc, parentElement);

    return actorElement;
}

void MeshActor::load(tinyxml2::XMLElement *actorElement)
{
    Actor::load(actorElement);

    rotationSpeedSlow = actorElement->FloatAttribute("slowSpeed");
    rotationSpeedFast = actorElement->FloatAttribute("fastSpeed");
    rotationSpeed = rotationSpeedFast;
}

} // namespace cmx
