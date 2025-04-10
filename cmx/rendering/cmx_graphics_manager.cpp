#include "cmx_graphics_manager.h"

// cmx
#include "IconsMaterialSymbols.h"
#include "cmx_camera.h"
#include "cmx_drawable.h"
#include "cmx_editor.h"
#include "cmx_frame_info.h"
#include "cmx_light_environment.h"
#include "cmx_material.h"
#include "cmx_render_system.h"
#include "cmx_texture.h"

// lib
#include <immintrin.h>
#include <spdlog/spdlog.h>

namespace cmx
{

std::vector<size_t> GraphicsManager::_shadowMapDescriptorSetIDs{};

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
    if (drawOption->material == nullptr)
    {
        return;
    }
    if (drawOption->material->editorOnly())
    {
#ifndef NDEBUG
        if (!Editor::isActive())
        {
            return;
        }
#endif
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
    FrameInfo *frameInfo = _renderSystem->beginCommandBuffer();
    if (!frameInfo)
        return;

    lightEnvironment->drawShadowMaps(frameInfo, _drawableRenderQueue, _shadowMapDescriptorSetIDs);

    if (Camera *camera = cameraWk.lock().get())
    {
        _noCameraFlag = false;

        _renderSystem->checkAspectRatio(camera);

        GlobalUbo ubo{};
        ubo.projection = camera->getProjection();
        ubo.view = camera->getView();

        if (lightEnvironment)
        {
            lightEnvironment->populateUbo(&ubo);
        }
        _renderSystem->writeUbo(frameInfo, &ubo);

        _renderSystem->beginRender(frameInfo, lightEnvironment);

        Material::resetBoundID();
        for (auto &[materialID, drawableQueue] : _drawableRenderQueue)
        {
            Texture::resetBoundID();
            for (auto &[drawable, drawOption] : drawableQueue)
            {
                if (drawable->isVisible())
                {
                    drawable->render(*frameInfo, drawOption);
                }
            }
        }

#ifndef NDEBUG
        if (Editor::isActive())
        {
            Editor *editor = Editor::getInstance();
            editor->render(*frameInfo);
        }
#endif
    }
    else
    {
        if (!_noCameraFlag)
        {
            spdlog::warn("GraphicsManager: No active camera");
            _noCameraFlag = true;
        }
    }

    _renderSystem->endRender();
}

void GraphicsManager::editor()
{
    int i = 0;

    for (auto &[materialID, drawableQueue] : _drawableRenderQueue)
    {
    }
}

const std::vector<size_t> &GraphicsManager::getDescriptorSetIDs()
{
    return _shadowMapDescriptorSetIDs;
}
} // namespace cmx
