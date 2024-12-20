#include "cmx_graphics_manager.h"

// cmx
#include "cmx_render_system.h"
#include <spdlog/spdlog.h>

namespace cmx
{

void GraphicsManager::addToQueue(std::shared_ptr<Component> component)
{
    componentRenderQueue.push_back(component);
}

void GraphicsManager::removeFromQueue(std::shared_ptr<Component> component)
{
    auto it = componentRenderQueue.begin();

    while (it != componentRenderQueue.end())
    {
        if (*it != component)
        {
            componentRenderQueue.erase(it);
            return;
        }
        it++;
    }
}

void GraphicsManager::drawComponents(std::weak_ptr<class CameraComponent> camera)
{
    renderSystem->drawScene(camera, componentRenderQueue);
}

} // namespace cmx
