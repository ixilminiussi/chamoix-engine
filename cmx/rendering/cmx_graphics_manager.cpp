#include "cmx_graphics_manager.h"

// cmx
#include "cmx_drawable.h"
#include "cmx_editor.h"
#include "cmx_frame_info.h"
#include "cmx_light_environment.h"
#include "cmx_material.h"
#include "cmx_render_system.h"
#include "cmx_texture.h"

// lib
#include <spdlog/spdlog.h>

namespace cmx
{

GraphicsManager::GraphicsManager() : _drawableRenderQueue{}
{
    _renderSystem = RenderSystem::getInstance();
}

void GraphicsManager::update(Drawable *drawable, DrawOption *drawOption, size_t oldID)
{
    if (drawable == nullptr)
    {
        throw("GraphicsManager: attempted to update empty drawable to graphics manager");
    }

    if (oldID != 0)
    {
        auto it = _drawableRenderQueue[oldID].begin();

        while (it != _drawableRenderQueue[oldID].end())
        {
            if (it->second == drawOption)
            {
                it = _drawableRenderQueue[oldID].erase(it);
            }
            else
            {
                it++;
            }
        }
    }

    add(drawable, drawOption);
}

void GraphicsManager::add(Drawable *drawable, DrawOption *drawOption)
{
    if (drawable == nullptr)
    {
        throw("GraphicsManager: attempted to add empty drawable to graphics manager");
    }

    size_t id = drawOption->getMaterialID();
    if (id == 0)
    {
        return;
    }

    _drawableRenderQueue[id].push_back({drawable, drawOption});
}

void GraphicsManager::remove(const DrawOption *drawOption)
{
    size_t id = drawOption->getMaterialID();
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
    const std::vector<DrawOption const *> drawOptions = drawable->getDrawOptions();

    for (const DrawOption *drawOption : drawOptions)
    {
        remove(drawOption);
    }
}

void GraphicsManager::drawRenderQueue(std::weak_ptr<Camera> cameraWk, const LightEnvironment *lightEnvironment)
{
    Material::resetBoundID();

    if (auto camera = cameraWk.lock().get())
    {
        _noCameraFlag = false;

        _renderSystem->checkAspectRatio(camera);
        FrameInfo *frameInfo = _renderSystem->beginRender(camera, lightEnvironment);

        for (auto &[materialID, drawableQueue] : _drawableRenderQueue)
        {
            Texture::resetBoundID();
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
