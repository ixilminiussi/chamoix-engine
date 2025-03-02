#include "cmx_graphics_manager.h"

// cmx
#include "cmx_drawable.h"
#include "cmx_editor.h"
#include "cmx_frame_info.h"
#include "cmx_light_environment.h"
#include "cmx_render_system.h"

// lib
#include <list>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace cmx
{

GraphicsManager::GraphicsManager()
{
    _renderSystem = RenderSystem::getInstance();
}

void GraphicsManager::update(Drawable *drawable, DrawOption *drawOption, unsigned int oldID)
{
    auto it = _drawableRenderQueue[oldID].begin();

    while (it != _drawableRenderQueue[oldID].end())
    {
        if (it->second == drawOption)
        {
            _drawableRenderQueue[oldID].erase(it);
            break;
        }
        it++;
    }

    add(drawable, drawOption);
}

void GraphicsManager::add(Drawable *drawable, DrawOption *drawOption)
{
    if (drawable == nullptr)
    {
        throw("attempted to add empty drawable to graphics manager");
    }

    unsigned int id = drawOption->getMaterialID();
    if (id == 0)
    {
        return;
    }

    _drawableRenderQueue[id].push_back({drawable, drawOption});
}

void GraphicsManager::remove(const DrawOption *drawOption)
{
    unsigned int id = drawOption->getMaterialID();
    auto it = _drawableRenderQueue[id].begin();

    while (it != _drawableRenderQueue[id].end())
    {
        if (it->second == drawOption)
        {
            _drawableRenderQueue[id].erase(it);
            return;
        }
        it++;
    }
}

void GraphicsManager::remove(const Drawable *drawable)
{
    const std::map<uint8_t, DrawOption> &drawOptions = drawable->getDrawOptions();

    for (const auto [discard, drawOption] : drawOptions)
    {
        remove(&drawOption);
    }
}

void GraphicsManager::drawRenderQueue(std::weak_ptr<Camera> cameraWk, const LightEnvironment *lightEnvironment)
{
    if (auto camera = cameraWk.lock().get())
    {
        _noCameraFlag = false;

        _renderSystem->checkAspectRatio(camera);
        const FrameInfo *frameInfo = _renderSystem->beginRender(camera, lightEnvironment);

        for (auto &[materialID, drawableQueue] : _drawableRenderQueue)
        {
            for (auto &[drawable, drawOption] : drawableQueue)
            {
                drawable->render(*frameInfo, drawOption);
            }
        }

#ifndef NDEBUG
        if (Editor::isActive())
        {
            Editor *editor = Editor::getInstance();
            editor->render(*frameInfo);
        }
#endif

        _renderSystem->endRender();
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
}

} // namespace cmx
