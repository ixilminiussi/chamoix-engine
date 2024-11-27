#include "triangle_actor.h"

#include "cmx_render_component.h"

// lib
#include <glm/ext/scalar_constants.hpp>
#include <spdlog/spdlog.h>

// std
#include <memory>

void TriangleActor::onBegin()
{
    transform.position = glm::vec2{0.f};
    transform.scale = glm::vec2{1.2f, 1.f};
    transform.rotation = .5f * glm::pi<float>();

    auto renderComponent = std::make_shared<cmx::RenderComponent>();
    attachComponent(renderComponent);
}

void TriangleActor::update(float dt)
{
    transform.rotation += 0.1 * dt;
}
