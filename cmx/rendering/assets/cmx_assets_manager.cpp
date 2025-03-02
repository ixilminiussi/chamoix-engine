#include "cmx_assets_manager.h"

// cmx
#include "cmx_game.h"
#include "cmx_hud_material.h"
#include "cmx_model.h"
#include "cmx_primitives.h"
#include "cmx_render_system.h"
#include "cmx_scene.h"
#include "cmx_texture.h"

// lib
#include "imgui.h"

// std
#include <cstdlib>

namespace cmx
{

AssetsManager::AssetsManager(class Scene *parent) : _parentScene{parent}
{
    addModel("assets/cmx/cube.obj", PRIMITIVE_CUBE);
    addModel("assets/cmx/cylinder.obj", PRIMITIVE_CYLINDER);
    addModel("assets/cmx/plane.obj", PRIMITIVE_PLANE);
    addModel("assets/cmx/sphere.obj", PRIMITIVE_SPHERE);
    addModel("assets/cmx/torus.obj", PRIMITIVE_TORUS);
    addModel("assets/cmx/camera.obj", "cmx_camera");
    addTexture("assets/cmx/missing-texture.png", "cmx_missing");
    addTexture("assets/cmx/point-light.png", "cmx_point_light");
};

tinyxml2::XMLElement &AssetsManager::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement)
{
    tinyxml2::XMLElement *assetsElement = doc.NewElement("assets");

    for (const auto &pair : _models)
    {
        tinyxml2::XMLElement &modelElement = pair.second->save(doc, assetsElement);
        modelElement.SetAttribute("name", pair.first.c_str());
    }

    for (const auto &pair : _textures)
    {
        tinyxml2::XMLElement &textureElement = pair.second->save(doc, assetsElement);
        textureElement.SetAttribute("name", pair.first.c_str());
    }

    parentElement->InsertEndChild(assetsElement);

    spdlog::info("Saved asset manager");

    return *assetsElement;
}

AssetsManager::~AssetsManager()
{
}

void AssetsManager::load(tinyxml2::XMLElement *parentElement)
{
    _materials["hud_material"] = std::make_unique<HudMaterial>();

    if (tinyxml2::XMLElement *assetsElement = parentElement->FirstChildElement("assets"))
    {
        tinyxml2::XMLElement *modelElement = assetsElement->FirstChildElement("model");
        while (modelElement)
        {
            addModel(modelElement->Attribute("filepath"), modelElement->Attribute("name"));

            modelElement = modelElement->NextSiblingElement("model");
        }

        tinyxml2::XMLElement *textureElement = assetsElement->FirstChildElement("texture");
        while (textureElement)
        {
            addTexture(textureElement->Attribute("filepath"), textureElement->Attribute("name"));

            textureElement = textureElement->NextSiblingElement("texture");
        }
    }
}

void AssetsManager::unload()
{
    spdlog::info("AssetsManager: Unloading assets manager...");
    {
        auto it = _models.begin();

        while (it != _models.end())
        {
            (*it).second->free();
            spdlog::info("AssetsManager: Unloaded model [{0}]", (*it).first);
            it++;
        }

        _models.clear();
    }

    {
        auto it = _textures.begin();

        while (it != _textures.end())
        {
            (*it).second->free();
            spdlog::info("AssetsManager: Unloaded texture [{0}]", (*it).first);
            it++;
        }

        _textures.clear();
    }
    spdlog::info("AssetsManager: Successfully unloaded assets manager!");
}

void AssetsManager::editor()
{
    ImGui::Button("testing");
}

void AssetsManager::addModel(const std::string &filepath, const std::string &name)
{
    if (_models.find(name) != _models.end())
    {
        spdlog::warn("AssetsManager: model of same name '{0}' already exists", name);
    }
    else
    {
        Device *device = RenderSystem::getDevice();
        if (device)
        {
            _models[name] = std::unique_ptr<Model>(Model::createModelFromFile(device, filepath, name));
        }
    }
}

void AssetsManager::removeModel(const std::string &name)
{
    // TODO: implement
}

void AssetsManager::addTexture(const std::string &filepath, const std::string &name)
{
    if (_textures.find(name) != _textures.end())
    {
        spdlog::warn("AssetsManager: texture of same name '{0}' already exists", name);
    }
    else
    {
        Device *device = RenderSystem::getDevice();
        if (device)
        {
            _textures[name] = std::unique_ptr<Texture>(Texture::createTextureFromFile(device, filepath, name));
        }
    }
}

void AssetsManager::removeTexture(const std::string &name)
{
    // TODO: implement
}

Model *AssetsManager::getModel(const std::string &name)
{
    if (_models.find(name) == _models.end())
    {
        spdlog::warn("AssetsManager: No such model named '{0}'", name);
        return nullptr;
    }

    return _models[name].get();
}

Texture *AssetsManager::getTexture(const std::string &name)
{
    if (_textures.find(name) == _textures.end())
    {
        spdlog::warn("AssetsManager: No such texture named '{0}'", name);
        return nullptr;
    }

    return _textures[name].get();
}

} // namespace cmx
