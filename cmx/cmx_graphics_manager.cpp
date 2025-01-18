#include "cmx_graphics_manager.h"

// cmx
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
    _pointLightsMap.reserve(MAX_POINT_LIGHTS);
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

void GraphicsManager::addPointLight(uint32_t id, PointLightStruct pointLight)
{
    if (_pointLightsMap.size() < MAX_POINT_LIGHTS)
    {
        _pointLightsMap[id] = pointLight;
    }
    else
    {
        spdlog::error("GraphicsManager: Reached maximum amount of point lights alloded by RenderSystem");
    }
}

void GraphicsManager::removePointLight(uint32_t id)
{
    _pointLightsMap.erase(id);
}

void GraphicsManager::drawComponents(std::weak_ptr<Camera> cameraWk)
{
    if (auto camera = cameraWk.lock().get())
    {
        _noCameraFlag = false;

        RenderSystem::checkAspectRatio(camera);

        PointLight pointLights[10];
        int numPointLights = _pointLightsMap.size();

        int i = 0;
        for (auto &pair : _pointLightsMap)
        {
            pointLights[i] = PointLight{glm::vec4(*pair.second.position, 1.0f),
                                        glm::vec4(*pair.second.lightColor, *pair.second.lightIntensity)};
            i++;
        }

        const FrameInfo *frameInfo = RenderSystem::beginRender(camera, pointLights, numPointLights);

        i = 0;
        for (auto &pair : _componentRenderQueue)
        {
            try
            {
                _renderSystems.at(pair.first)->render(frameInfo, pair.second, this);
            }
            catch (const std::out_of_range &e)
            {
                spdlog::error("GraphicsManager: Component requesting render system {0} which isn't part of graphics "
                              "manager's list",
                              pair.first);
            }
        }

#ifndef NDEBUG
        if (CmxEditor::isActive())
        {
            CmxEditor *editor = CmxEditor::getInstance();
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
