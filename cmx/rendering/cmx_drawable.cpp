#include "cmx_drawable.h"

// cmx
#include "cmx_actor.h"
#include "cmx_frame_info.h"
#include "cmx_graphics_manager.h"
#include "cmx_material.h"
#include "cmx_model.h"
#include "cmx_texture.h"
#include <list>

namespace cmx
{

Drawable::Drawable(Actor **parentP) : _parentP{parentP}
{
    if (_parentP == nullptr)
    {
        throw("parentP cannot be nullptr");
    }
}

Drawable::~Drawable()
{
}

void Drawable::setDrawOption(const DrawOption &drawOption, uint8_t index)
{
    unsigned int oldID = _drawOptions[index].getMaterialID();

    _drawOptions[index] = drawOption;

    if (oldID == 0)
    {
        getParentActor()->getScene()->getGraphicsManager()->add(this, &_drawOptions[index]);
    }

    getParentActor()->getScene()->getGraphicsManager()->update(this, &_drawOptions[index], oldID);
}

void Drawable::setMaterial(class Material *material, uint8_t index)
{
    unsigned int oldID = _drawOptions[index].getMaterialID();

    _drawOptions[index].material = material;

    getParentActor()->getScene()->getGraphicsManager()->update(this, &_drawOptions[index], oldID);
}

void Drawable::setTextures(const std::vector<class Texture *> textures, uint8_t index)
{
    _drawOptions[index].textures = textures;
}

void Drawable::setModel(Model *model, uint8_t index)
{
    _drawOptions[index].model = model;
}

unsigned int DrawOption::getMaterialID() const
{
    if (material == nullptr)
    {
        return 0;
    }

    return material->getID();
}

void Drawable::editor()
{
}

void Drawable::load()
{
}

void Drawable::save()
{
}

void Drawable::render(const FrameInfo &frameInfo, DrawOption *drawOption) const
{
    drawOption->material->bind(frameInfo.commandBuffer);

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

const std::map<uint8_t, DrawOption> &Drawable::getDrawOptions() const
{
    return _drawOptions;
}

} // namespace cmx
