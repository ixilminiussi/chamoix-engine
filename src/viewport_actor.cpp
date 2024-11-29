#include "cmx_camera_component.h"

#include "cmx_input_manager.h"
#include "game.h"
#include "viewport_actor.h"

// lib
#include <spdlog/spdlog.h>

// std
#include <memory>

void ViewportActor::onBegin()
{
    camera = std::make_shared<cmx::CmxCameraComponent>();
    camera->setViewDirection(glm::vec3{0.f}, glm::vec3{.5, 0.f, 1.f});

    getWorld()->getGame()->getInputManager().bind(
        "forward_move", std::bind(&ViewportActor::onForwardInputPressed, this, std::placeholders::_1));
}

void ViewportActor::update(float dt)
{
}

void ViewportActor::onForwardInputPressed(bool status)
{
    if (status)
    {
        spdlog::info("key was pressed");
    }
    else
    {
        spdlog::info("key was released");
    }
}
