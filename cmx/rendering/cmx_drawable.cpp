#include "cmx_drawable.h"

// cmx
#include "cmx_actor.h"
#include "cmx_assets_manager.h"
#include "cmx_frame_info.h"
#include "cmx_graphics_manager.h"
#include "cmx_material.h"
#include "cmx_model.h"
#include "cmx_render_system.h"
#include "cmx_texture.h"
#include "cmx_utils.h"

// lib
#include <IconsMaterialSymbols.h>
#include <SPIRV-Reflect/spirv_reflect.h>
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

    if (drawOption.textures.size() > drawOption.material->getTotalSamplers())
    {
        spdlog::error("Drawable: too many textures assigned to a material [{0}] which does not support it",
                      drawOption.material->name);
    }

    getParentActor()->getScene()->getGraphicsManager()->add(this, &_drawOptions[index]);
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
                const char *selected;

                selected = drawOption.material != nullptr ? drawOption.material->name.c_str() : "";

                ImGui::Text("Material:");
                if (drawOption.material != nullptr)
                {
                    if (ImGui::Button(ICON_MS_CONTENT_COPY))
                    {
                        Material *duplicate = drawOption.material->clone();
                        assetsManager->addMaterial(duplicate, incrementNumberInParentheses(drawOption.material->name));
                        setMaterial(duplicate->name);

                        ImGui::SetNextItemWidth(170);
                        ImGui::SameLine();
                    }
                }
                if (ImGui::BeginCombo("##Material", selected))
                {
                    for (const auto &pair : assetsManager->getMaterials())
                    {
                        bool isSelected = (strcmp(selected, pair.first.c_str()) == 0);
                        if (ImGui::Selectable(pair.first.c_str(), isSelected) &&
                            strcmp(selected, pair.first.c_str()) != 0)
                        {
                            selected = pair.first.c_str();
                            setMaterial(pair.first, id);
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
                                    setModel(pair.first, id);
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
                    const std::vector<BindingInfo> &bindings = drawOption.material->getBindings();
                    int textureIndex = 0;
                    for (int i = 0; i < bindings.size(); i++)
                    {
                        if (bindings[i].type == SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER)
                        {
                            selected = drawOption.textures[textureIndex] != nullptr
                                           ? drawOption.textures[textureIndex]->name.c_str()
                                           : "";

                            ImGui::PushID(i);
                            if (ImGui::BeginCombo(std::to_string(bindings[i].binding).c_str(), selected))
                            {
                                for (const auto &pair : assetsManager->getTextures())
                                {
                                    bool isSelected = (strcmp(selected, pair.first.c_str()) == 0);

                                    if (ImGui::Selectable(pair.first.c_str(), isSelected))
                                    {
                                        selected = pair.first.c_str();
                                        drawOption.textures[textureIndex] = assetsManager->getTexture(pair.first);
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

            for (int i = 0; i < std::min(option.material->getTotalSamplers(), option.textures.size()); i++)
            {
                optionElement->SetAttribute(("t" + std::to_string(i)).c_str(), option.textures[i]->name.c_str());
            }
            drawableElement->InsertEndChild(optionElement);
        }
    }
    parentElement->InsertEndChild(drawableElement);

    return *drawableElement;
}

void Drawable::load(tinyxml2::XMLElement *drawableElement)
{
    tinyxml2::XMLElement *optionElement = drawableElement->FirstChildElement();

    while (optionElement)
    {
        size_t index = optionElement->IntAttribute("index");

        setDrawOption(DrawOption{}, index);

        setMaterial(optionElement->Attribute("material", nullptr), index);

        if (_drawOptions[index].material->needsModel())
        {
            setModel(optionElement->Attribute("model", nullptr), index);
        }

        std::vector<std::string> textureNames{};
        for (int i = 0; i < _drawOptions[i].material->getTotalSamplers(); i++)
        {
            std::string name = optionElement->Attribute(("option" + std::to_string(i)).c_str(), "null");
            if (name.compare("null") == 0)
            {
                break;
            }
            textureNames.push_back(name);
        }
        setTextures(textureNames);

        optionElement = drawableElement->NextSiblingElement();
    }
}

void Drawable::render(FrameInfo &frameInfo, DrawOption *drawOption) const
{
    size_t textureCount = drawOption->material->getTotalSamplers();
    if (drawOption->textures.size() < textureCount)
        return;
    if (drawOption->material->needsModel() && drawOption->model == nullptr)
        return;

    drawOption->material->bind(&frameInfo, this);
    for (int i = 0; i < textureCount; i++)
    {
        drawOption->textures[i]->bind(frameInfo.commandBuffer, drawOption->material->getPipelineLayout());
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
