#include "cmx_graphics_manager.h"

// cmx
#include "cmx/cmx_frame_info.h"
#include "cmx_render_system.h"

// lib
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace cmx
{

GraphicsManager::GraphicsManager(std::unordered_map<uint8_t, std::shared_ptr<RenderSystem>> &renderSystems)
    : _renderSystems{renderSystems}
{
}

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

void GraphicsManager::drawComponents(std::weak_ptr<Camera> cameraWk)
{
    if (auto camera = cameraWk.lock().get())
    {
        _noCameraFlag = false;

        RenderSystem::checkAspectRatio(camera);
        FrameInfo *frameInfo = RenderSystem::beginRender(camera);

        for (auto component : _componentRenderQueue)
        {
            try
            {
                if (component->getVisible())
                {
                    _renderSystems.at(component->getRequestedRenderSystem())->render(frameInfo, component);
                }
            }
            catch (std::out_of_range e)
            {
                spdlog::error("GraphicsManager: Component requesting render system {0} which isn't part of graphics "
                              "manager's list",
                              component->getRequestedRenderSystem());
            }
        }

        RenderSystem::endRender();
    }
    else
    {
        if (!_noCameraFlag)
        {
            spdlog::warn("GraphicsManager: No active camera");
            _noCameraFlag = true;
        }
    }
}

} // namespace cmx
