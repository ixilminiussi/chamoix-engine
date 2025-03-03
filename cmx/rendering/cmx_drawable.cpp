#include "cmx_drawable.h"

// cmx
#include "cmx_actor.h"
#include "cmx_frame_info.h"
#include "cmx_graphics_manager.h"
#include "cmx_material.h"
#include "cmx_model.h"
#include "cmx_texture.h"
#include <cstdio>
#include <list>
#include <string>

namespace cmx
{

Drawable::Drawable(Actor **parentP) : _parentP{parentP}
{
    if (_parentP == nullptr)
    {
        throw("parentP cannot be nullptr");
    }
}

void Drawable::setDrawOption(const DrawOption &drawOption, size_t index)
{
    unsigned int oldID = _drawOptions[index].getMaterialID();

    _drawOptions[index] = drawOption;

    if (oldID == 0)
    {
        getParentActor()->getScene()->getGraphicsManager()->add(this, &_drawOptions[index]);
    }

    getParentActor()->getScene()->getGraphicsManager()->update(this, &_drawOptions[index], oldID);
}

void Drawable::setMaterial(const std::string &name, size_t index)
{
    unsigned int oldID = _drawOptions[index].getMaterialID();

    AssetsManager *assetsManager = getParentActor()->getScene()->getAssetsManager();
    _drawOptions[index].material = assetsManager->getMaterial(name);

    getParentActor()->getScene()->getGraphicsManager()->update(this, &_drawOptions[index], oldID);
}

void Drawable::setTextures(const std::vector<std::string> textures, size_t index)
{
    AssetsManager *assetsManager = getParentActor()->getScene()->getAssetsManager();

    _drawOptions[index].textures.clear();

    for (const std::string &name : textures)
    {
        _drawOptions[index].textures.push_back(assetsManager->getTexture(name));
    }
}

void Drawable::setModel(const std::string &name, size_t index)
{
    AssetsManager *assetsManager = getParentActor()->getScene()->getAssetsManager();
    _drawOptions[index].model = assetsManager->getModel(name);
}

unsigned int DrawOption::getMaterialID() const
{
    if (material == nullptr)
    {
        return 0;
    }

    return material->getID();
}

void Drawable::editor(int i)
{
    if (ImGui::TreeNode("Rendering"))
    {
        AssetsManager *assetsManager = getParentActor()->getScene()->getAssetsManager();
        for (auto &[id, drawOption] : _drawOptions)
        {
            if (ImGui::TreeNode(std::to_string(id).c_str()))
            {
                const char *selected = drawOption.material != nullptr ? drawOption.material->name.c_str() : "";

                if (ImGui::BeginCombo("Material##", selected))
                {
                    for (const auto &pair : assetsManager->getMaterials())
                    {
                        bool isSelected = (strcmp(selected, pair.first.c_str()) == 0);

                        if (ImGui::Selectable(pair.first.c_str(), isSelected))
                        {
                            selected = pair.first.c_str();
                            setMaterial(pair.first);
                        }

                        if (isSelected)
                        {
                            ImGui::SetItemDefaultFocus();
                        }
                    }

                    ImGui::EndCombo();
                }
            }
        }
    }
}

void Drawable::load()
{
}

void Drawable::save()
{
}

void Drawable::render(FrameInfo &frameInfo, DrawOption *drawOption) const
{
    drawOption->material->bind(&frameInfo, this);

    for (Texture *texture : drawOption->textures)
    {
        texture->bind(frameInfo.commandBuffer, drawOption->material->getPipelineLayout());
    }

    if (drawOption->model != nullptr)
    {
        drawOption->model->bind(frameInfo.commandBuffer);
        drawOption->model->draw(frameInfo.commandBuffer);
    }
    else
    {
        frameInfo.commandBuffer.draw(6, 1, 0, 0);
    }
}

const std::map<size_t, DrawOption> &Drawable::getDrawOptions() const
{
    return _drawOptions;
}

} // namespace cmx
