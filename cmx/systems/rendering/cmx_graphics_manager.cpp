#include "cmx_graphics_manager.h"

// cmx
#include "cmx_render_system.h"

// lib
#include <spdlog/spdlog.h>

namespace cmx
{

void GraphicsManager::addToQueue(std::shared_ptr<Component> component)
{
    _componentRenderQueue.push_back(component);
}

void GraphicsManager::removeFromQueue(std::shared_ptr<Component> component)
{
    auto it = _componentRenderQueue.begin();

    while (it != _componentRenderQueue.end())
    {
        if (*it != component)
        {
            _componentRenderQueue.erase(it);
            return;
        }
        it++;
    }
}

void GraphicsManager::drawComponents(std::weak_ptr<CameraComponent> camera)
{
    _renderSystem->drawScene(camera, _componentRenderQueue);
}

} // namespace cmx
