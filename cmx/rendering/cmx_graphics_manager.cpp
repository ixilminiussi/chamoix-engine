#include "cmx_graphics_manager.h"

// cmx
#include "cmx_assets_manager.h"
#include "cmx_camera.h"
#include "cmx_drawable.h"
#include "cmx_editor.h"
#include "cmx_frame_info.h"
#include "cmx_light_environment.h"
#include "cmx_material.h"
#include "cmx_post_outline_material.h"
#include "cmx_post_passthrough_material.h"
#include "cmx_render_system.h"
#include "cmx_texture.h"
#include "cmx_utils.h"
#include "imgui.h"

// lib
#include <IconsMaterialSymbols.h>
#include <immintrin.h>
#include <spdlog/spdlog.h>

namespace cmx
{

std::vector<size_t> GraphicsManager::_shadowMapDescriptorSetIDs{};

GraphicsManager::GraphicsManager() : _drawableRenderQueue{}
{
    _renderSystem = RenderSystem::getInstance();
    _postProcessMaterials = {new PostPassthroughMaterial()};

    for (Material *material : _postProcessMaterials)
    {
        material->initialize();
    }
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

void GraphicsManager::drawRenderQueue(std::weak_ptr<Camera> cameraWk, LightEnvironment *graphicsManager)
{
    FrameInfo *frameInfo = _renderSystem->beginCommandBuffer();
    if (!frameInfo)
        return;

    graphicsManager->drawShadowMaps(frameInfo, _drawableRenderQueue, _shadowMapDescriptorSetIDs);

    if (Camera *camera = cameraWk.lock().get())
    {
        _noCameraFlag = false;

        frameInfo->camera = camera;
        _renderSystem->checkAspectRatio(camera);

        GlobalUbo ubo{};
        ubo.projection = camera->getProjection();
        ubo.view = camera->getView();
        ubo.cameraPos = glm::vec4(camera->getPosition(), 1.0f);

        if (graphicsManager)
        {
            graphicsManager->populateUbo(&ubo);
        }
        _renderSystem->writeUbo(frameInfo, &ubo);

        _renderSystem->beginRender(frameInfo, graphicsManager);

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
    }
    else
    {
        if (!_noCameraFlag)
        {
            spdlog::warn("GraphicsManager: No active camera");
            _noCameraFlag = true;
        }
    }

    _renderSystem->endRender(frameInfo);

    _renderSystem->beginPostProcess(frameInfo);

    for (Material *material : _postProcessMaterials)
    {
        material->bind(frameInfo, nullptr);
        frameInfo->commandBuffer.draw(6, 1, 0, 0);
    }
    _renderSystem->endPostProcess(frameInfo);
}

void GraphicsManager::editor(AssetsManager *assetsManager)
{
    auto it = _postProcessMaterials.begin();
    int i = 0;

    while (it != _postProcessMaterials.end())
    {
        ImGui::PushID(i++);
        Material *material = *it;
        if (material)
        {
            ImGui::Text("%s", material->name.c_str());
            if (i > 0)
            {
                ImGui::SameLine();
                if (ImGui::Button(ICON_MS_DELETE))
                {
                    _postProcessMaterials.erase(it);
                    ImGui::PopID();
                    continue;
                }
            }
            material->editor();
        }

        it++;
        ImGui::PopID();
    }

    // add new post process material
    static const char *selected = assetsManager->getPostProcesses().begin()->first.c_str();

    ImGui::SeparatorText("New Post Process");
    ImGui::PushID(i++);
    ImGui::SetNextItemWidth(170);
    if (ImGui::BeginCombo("##Material", selected))
    {
        for (const auto &[name, material] : assetsManager->getPostProcesses())
        {
            bool isSelected = (strcmp(selected, name.c_str()) == 0);

            if (ImGui::Selectable(name.c_str(), isSelected) && !isSelected)
            {
                selected = name.c_str();
                isSelected = true;
            }

            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }
    ImGui::PopID();

    ImGui::SameLine();
    if (ImGui::Button(ICON_MS_ADD))
    {
        Material *material = assetsManager->getPostProcess(selected);
        addPostProcess(material);
    }
}

tinyxml2::XMLElement &GraphicsManager::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const
{
    tinyxml2::XMLElement *graphicsManagerElement = doc.NewElement("graphicsManager");

    int i = -1;
    for (Material *material : _postProcessMaterials)
    {
        i++;
        if (i == 0)
        {
            continue;
        }

        graphicsManagerElement->SetAttribute(("pp" + std::to_string(i)).c_str(), material->name.c_str());
    }

    parentElement->InsertEndChild(graphicsManagerElement);

    return *graphicsManagerElement;
}

void GraphicsManager::load(tinyxml2::XMLElement *parentElement, AssetsManager *assetsManager)
{
    if (tinyxml2::XMLElement *graphicsManagerElement = parentElement->FirstChildElement("graphicsManager"))
    {
        int i = 0;
        while (true)
        {
            i++;
            std::string attributeName = "pp" + std::to_string(i);
            const char *name = graphicsManagerElement->Attribute(attributeName.c_str());
            if (name != 0)
            {
                Material *material = assetsManager->getPostProcess(name);
                addPostProcess(material);
            }
            else
            {
                break;
            }
        }
    }
}

void GraphicsManager::addPostProcess(Material *material)
{
    if (material == nullptr)
    {
        return;
    }

    _postProcessMaterials.push_back(material);
}

const std::vector<size_t> &GraphicsManager::getDescriptorSetIDs()
{
    return _shadowMapDescriptorSetIDs;
}
} // namespace cmx
