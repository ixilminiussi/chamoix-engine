#include "cmx_graphics_manager.h"

// cmx
#include "cmx/cmx_light_environment.h"
#include "cmx_actor.h"
#include "cmx_editor.h"
#include "cmx_frame_info.h"
#include "cmx_render_system.h"

// lib
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace cmx
{

GraphicsManager::GraphicsManager(std::map<uint8_t, std::shared_ptr<RenderSystem>> &renderSystems)
    : _renderSystems{renderSystems}
{
}

void GraphicsManager::addToQueue(std::shared_ptr<Component> component)
{
    uint8_t renderSystemID = component->getRequestedRenderSystem();

    try
    {
        auto &renderQueue = _renderSystems.at(renderSystemID);
    }
    catch (std::out_of_range e)
    {
        spdlog::error("GraphicsManager: Requesting non-existant render system '{0}'", renderSystemID);
        return;
    }

    auto it = std::lower_bound(_componentRenderQueue[renderSystemID].begin(),
                               _componentRenderQueue[renderSystemID].end(), component);
    _componentRenderQueue[renderSystemID].insert(it, component);
}

void GraphicsManager::removeFromQueue(std::shared_ptr<Component> component)
{
    uint8_t renderSystemID = component->getRequestedRenderSystem();
    auto it = _componentRenderQueue[renderSystemID].begin();

    while (it != _componentRenderQueue[renderSystemID].end())
    {
        if (*it == component)
        {
            _componentRenderQueue[renderSystemID].erase(it);
            return;
        }
        it++;
    }
}

void GraphicsManager::drawComponents(std::weak_ptr<Camera> cameraWk, const LightEnvironment *lightEnvironment)
{
    if (auto camera = cameraWk.lock().get())
    {
        _noCameraFlag = false;

        RenderSystem::checkAspectRatio(camera);
        const FrameInfo *frameInfo = RenderSystem::beginRender(camera, lightEnvironment);

        for (auto &pair : _componentRenderQueue)
        {
            try
            {
                _renderSystems.at(pair.first)->render(frameInfo, pair.second, this);
            }
            catch (const std::out_of_range &)
            {
                spdlog::error("GraphicsManager: Component requesting render system {0} which isn't part of graphics "
                              "manager's list",
                              pair.first);
            }
        }

#ifndef NDEBUG
        if (Editor::isActive())
        {
            Editor *editor = Editor::getInstance();
            editor->render(*frameInfo);
        }
#endif

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

void GraphicsManager::editor()
{
    int i = 0;

    for (auto pair : _renderSystems)
    {
        pair.second->editor(i++);
    }
}

} // namespace cmx
