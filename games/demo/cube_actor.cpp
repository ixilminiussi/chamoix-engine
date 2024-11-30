#include "cube_actor.h"
#include "cmx_render_component.h"
#include <glm/ext/scalar_constants.hpp>

void CubeActor::onBegin()
{
    transform.position = {0.f, 0.f, 5.f};
    transform.scale = {.5f, .5f, .5f};
    transform.rotation = {0.f, 0.f, 0.f};

    auto renderComponent = std::make_shared<cmx::RenderComponent>();
    attachComponent(renderComponent);
}

void CubeActor::update(float dt)
{
    transform.rotation.x += dt * glm::pi<float>();
    transform.rotation.y += dt * glm::pi<float>();
}
