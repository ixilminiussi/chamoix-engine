#include "cmx_camera_component.h"

// cmx
#include "cmx_actor.h"
#include "cmx_camera.h"

namespace cmx
{

CameraComponent::CameraComponent() : _camera{std::make_shared<Camera>()} {};

void CameraComponent::update(float dt)
{
    _camera->setViewDirection(getParent()->transform.position, getParent()->transform.forward(),
                              getParent()->transform.up());
}

tinyxml2::XMLElement &CameraComponent::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentComponent)
{
    tinyxml2::XMLElement &cameraComponent = Component::save(doc, parentComponent);
    cameraComponent.SetAttribute("fov", _camera->getFOV());
    cameraComponent.SetAttribute("nearPlane", _camera->getNearPlane());
    cameraComponent.SetAttribute("farPlane", _camera->getFarPlane());

    parentComponent->InsertEndChild(&cameraComponent);

    return cameraComponent;
}

} // namespace cmx
