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

} // namespace cmx
