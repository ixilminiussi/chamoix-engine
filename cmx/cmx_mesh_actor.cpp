#include "cmx_mesh_actor.h"

// cmx
#include "cmx_mesh_component.h"

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
    Actor::editor();
}

tinyxml2::XMLElement &MeshActor::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement)
{
    tinyxml2::XMLElement &actorElement = Actor::save(doc, parentElement);

    return actorElement;
}

void MeshActor::load(tinyxml2::XMLElement *actorElement)
{
    Actor::load(actorElement);
}

} // namespace cmx
