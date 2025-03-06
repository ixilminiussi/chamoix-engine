#include "cmx_drawable.h"

// cmx
#include "cmx_actor.h"
#include "cmx_assets_manager.h"
#include "cmx_editor.h"
#include "cmx_frame_info.h"
#include "cmx_graphics_manager.h"
#include "cmx_material.h"
#include "cmx_model.h"
#include "cmx_render_system.h"
#include "cmx_texture.h"
#include "cmx_utils.h"

// lib
#include <IconsMaterialSymbols.h>
#include <SPIRV-Reflect/include/spirv/unified1/spirv.h>
#include <SPIRV-Reflect/spirv_reflect.h>
#include <algorithm>
#include <cstring>
#include <imgui.h>

// std
#include <cstdio>
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
    auto existingIT = _drawOptions.find(index);
    if (existingIT != _drawOptions.end())
    {
        getParentActor()->getScene()->getGraphicsManager()->remove(&existingIT->second);
    }

    _drawOptions[index] = drawOption;

    if (drawOption.material == nullptr)
    {
        return;
    }

    if (drawOption.textures.size() > drawOption.material->getRequestedSamplerCount())
    {
        spdlog::warn("Drawable: {0} textures assigned to a material [{1}] which supports {2}",
                     drawOption.textures.size(), drawOption.material->name,
                     drawOption.material->getRequestedSamplerCount());
    }

    getParentActor()->getScene()->getGraphicsManager()->add(this, &_drawOptions[index]);
}

void Drawable::setMaterial(const char *name, size_t index)
{
    size_t oldID = _drawOptions[index].getMaterialID();

    AssetsManager *assetsManager = getParentActor()->getScene()->getAssetsManager();
    _drawOptions[index].material = assetsManager->getMaterial(name);

    getParentActor()->getScene()->getGraphicsManager()->update(this, &_drawOptions[index], oldID);
}

void Drawable::setTextures(const std::vector<const char *> textures, size_t index)
{
    AssetsManager *assetsManager = getParentActor()->getScene()->getAssetsManager();

    _drawOptions[index].textures.clear();

    for (const char *name : textures)
    {
        _drawOptions[index].textures.push_back(assetsManager->getAnyTexture(name));
    }
}

void Drawable::setModel(const char *name, size_t index)
{
    AssetsManager *assetsManager = getParentActor()->getScene()->getAssetsManager();
    _drawOptions[index].model = assetsManager->getModel(name);
}

size_t DrawOption::getMaterialID() const
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
                const char *selected;

                selected = drawOption.material != nullptr ? drawOption.material->name.c_str() : "";

                ImGui::Text("Material:");
                if (drawOption.material != nullptr)
                {
                    if (ImGui::Button(ICON_MS_CONTENT_COPY))
                    {
                        Material *duplicate = assetsManager->makeUnique(drawOption.material->name.c_str());
                        setMaterial(duplicate->name.c_str(), id);

                        ImGui::SetNextItemWidth(170);
                        ImGui::SameLine();
                    }
                }
                if (ImGui::BeginCombo("##Material", selected))
                {
                    for (const auto &pair : assetsManager->getMaterials())
                    {
                        bool isSelected = (strcmp(selected, pair.first.c_str()) == 0);
                        if (ImGui::Selectable(pair.first.c_str(), isSelected) && !isSelected)
                        {
                            selected = pair.first.c_str();
                            setMaterial(selected, id);
                            isSelected = true;
                        }

                        if (isSelected)
                        {
                            ImGui::SetItemDefaultFocus();
                        }
                    }

                    ImGui::EndCombo();
                }

                if (drawOption.material != nullptr)
                {
                    ImGui::Text("Parameters:");
                    drawOption.material->editor();

                    if (drawOption.material->needsModel())
                    {
                        selected = drawOption.model != nullptr ? drawOption.model->name.c_str() : "";

                        ImGui::Text("Model:");
                        if (ImGui::BeginCombo("##Model", selected))
                        {
                            for (const auto &pair : assetsManager->getModels())
                            {
                                bool isSelected = (strcmp(selected, pair.first.c_str()) == 0);

                                if (ImGui::Selectable(pair.first.c_str(), isSelected))
                                {
                                    selected = pair.first.c_str();
                                    setModel(selected, id);
                                }

                                if (isSelected)
                                {
                                    ImGui::SetItemDefaultFocus();
                                }
                            }

                            ImGui::EndCombo();
                        }
                    }

                    ImGui::Text("Textures:");
                    const std::set<BindingInfo> &bindings = drawOption.material->getBindings();
                    int textureIndex = 0;

                    for (const BindingInfo &binding : bindings)
                    {
                        if (binding.type == SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
                        {
                            if (textureIndex < drawOption.textures.size())
                            {
                                selected = drawOption.textures[textureIndex] != nullptr
                                               ? drawOption.textures[textureIndex]->name.c_str()
                                               : "";
                            }
                            else
                            {
                                selected = "";
                            }

                            ImGui::PushID(textureIndex);
                            if (ImGui::BeginCombo(std::to_string(binding.binding).c_str(), selected))
                            {
                                const std::map<std::string, std::unique_ptr<Texture>> &textureRegister =
                                    binding.dim == SpvDim2D ? assetsManager->get2DTextures()
                                    : binding.dim == SpvDim3D
                                        ? assetsManager->get3DTextures()
                                        : []() -> const std::map<std::string, std::unique_ptr<Texture>> & {
                                    static const std::map<std::string, std::unique_ptr<Texture>> emptyMap;
                                    return emptyMap;
                                }();
                                for (const auto &pair : textureRegister)
                                {
                                    bool isSelected = (strcmp(selected, pair.first.c_str()) == 0);

                                    if (ImGui::Selectable(pair.first.c_str(), isSelected))
                                    {
                                        selected = pair.first.c_str();
                                        drawOption.textures.resize(
                                            std::max((size_t)textureIndex + 1, drawOption.textures.size()));
                                        drawOption.textures[textureIndex] = binding.dim == SpvDim2D
                                                                                ? assetsManager->get2DTexture(selected)
                                                                                : assetsManager->get3DTexture(selected);
                                    }

                                    if (isSelected)
                                    {
                                        ImGui::SetItemDefaultFocus();
                                    }
                                }

                                ImGui::EndCombo();
                            }
                            ImGui::PopID();
                            textureIndex++;
                        }
                    }
                }

                ImGui::TreePop();
            }
        }
        if (ImGui::Button(ICON_MS_ADD))
        {
        }
        ImGui::TreePop();
    }
}

tinyxml2::XMLElement &Drawable::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const
{
    tinyxml2::XMLElement *drawableElement = doc.NewElement("rendering");

    for (auto &[index, option] : _drawOptions)
    {
        if (option.material != nullptr)
        {
            tinyxml2::XMLElement *optionElement = doc.NewElement("option");
            optionElement->SetAttribute("index", index);
            optionElement->SetAttribute("material", option.material->name.c_str());

            if (option.model != nullptr && option.material->needsModel())
            {
                optionElement->SetAttribute("model", option.model->name.c_str());
            }

            for (int i = 0; i < std::min(option.material->getRequestedSamplerCount(), option.textures.size()); i++)
            {
                optionElement->SetAttribute(("t" + std::to_string(i)).c_str(), option.textures[i]->name.c_str());
            }
            drawableElement->InsertEndChild(optionElement);
        }
    }
    parentElement->InsertEndChild(drawableElement);

    return *drawableElement;
}

void Drawable::load(tinyxml2::XMLElement *parentElement)
{
    if (tinyxml2::XMLElement *drawableElement = parentElement->FirstChildElement("rendering"))
    {
        tinyxml2::XMLElement *optionElement = drawableElement->FirstChildElement();

        while (optionElement)
        {
            int i = 0;
            size_t index = optionElement->IntAttribute("index");

            setDrawOption(DrawOption{}, index);
            setMaterial(optionElement->Attribute("material"), index);

            if (_drawOptions[index].material->needsModel())
            {
                setModel(optionElement->Attribute("model"), index);
            }

            size_t totalSamplerCount = _drawOptions[i].material->getRequestedSamplerCount();
            std::vector<const char *> textureNames{};
            for (int i = 0; i < totalSamplerCount; i++)
            {
                std::string attributeName = "t" + std::to_string(i);
                const char *name = optionElement->Attribute(attributeName.c_str());
                if (name != 0)
                {
                    textureNames.push_back(name);
                }
            }
            setTextures(textureNames);

            optionElement = drawableElement->NextSiblingElement();
        }
    }
}

void Drawable::render(FrameInfo &frameInfo, DrawOption *drawOption) const
{
    if (drawOption->material == nullptr)
        return;
    if (!drawOption->material->isVisible())
        return;
    if (!(*_parentP)->isVisible())
        return;

    size_t textureCount = drawOption->material->getRequestedSamplerCount();

    drawOption->textures.resize(std::min(textureCount, drawOption->textures.size()));

    if (drawOption->textures.size() < textureCount)
        return;
    if (drawOption->material->needsModel() && drawOption->model == nullptr)
        return;

    drawOption->material->bind(&frameInfo, this);
    if (textureCount > 1)
    {
        Texture::bind(frameInfo.commandBuffer, drawOption->material->getPipelineLayout(), drawOption->textures);
    }
    if (textureCount == 1)
    {
        drawOption->textures[0]->bind(frameInfo.commandBuffer, drawOption->material->getPipelineLayout());
    }

    if (drawOption->material->needsModel())
    {
        drawOption->model->bind(frameInfo.commandBuffer);
        drawOption->model->draw(frameInfo.commandBuffer);
    }
    else
    {
        frameInfo.commandBuffer.draw(6, 1, 0, 0);
    }
}

const std::vector<DrawOption const *> Drawable::getDrawOptions() const
{
    std::vector<DrawOption const *> options;

    auto it = _drawOptions.begin();

    while (it != _drawOptions.end())
    {
        options.push_back(&(it->second));
        it++;
    }

    return options;
}

} // namespace cmx
