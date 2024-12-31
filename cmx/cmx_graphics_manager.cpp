#include "cmx_graphics_manager.h"

// cmx
#include "cmx_render_system.h"

// lib
#include <spdlog/spdlog.h>

namespace cmx
{

void GraphicsManager::addToQueue(std::shared_ptr<Component> component)
{
    auto it = std::lower_bound(_componentRenderQueue.begin(), _componentRenderQueue.end(), component);
    _componentRenderQueue.insert(it, component);
}

void GraphicsManager::removeFromQueue(std::shared_ptr<Component> component)
{
    auto it = _componentRenderQueue.begin();

    while (it != _componentRenderQueue.end())
    {
        if (*it == component)
        {
            _componentRenderQueue.erase(it);
            return;
        }
        it++;
    }
}

void GraphicsManager::drawComponents(std::weak_ptr<Camera> camera)
{
    _renderSystem->drawScene(camera, _componentRenderQueue);
}

} // namespace cmx
