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
    _meshComponent = std::make_shared<MeshComponent>();
    attachComponent(_meshComponent);
}

void MeshActor::update(float dt)
{
}

void MeshActor::updateMesh(std::string &meshName)
{
    _meshComponent->setModel(meshName);
}

void MeshActor::renderSettings()
{
    Actor::renderSettings();
}

tinyxml2::XMLElement &MeshActor::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement)
{
    tinyxml2::XMLElement &actorElement = Actor::save(doc, parentElement);

    actorElement.SetAttribute("mesh", _meshComponent->getModelName().c_str());

    return actorElement;
}

void MeshActor::load(tinyxml2::XMLElement *actorElement)
{
    Actor::load(actorElement);

    std::string meshName = actorElement->Attribute("mesh");
    updateMesh(meshName);
}

} // namespace cmx
