#include "cmx_camera_component.h"

#include "viewport_actor.h"

// std
#include <memory>

void ViewportActor::onBegin()
{
    camera = std::make_shared<cmx::CmxCameraComponent>();
    camera->setViewDirection(glm::vec3{0.f}, glm::vec3{.5, 0.f, 1.f});
}

void ViewportActor::update(float dt)
{
}
