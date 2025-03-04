#include "cmx_assets_manager.h"

// cmx
#include "cmx_billboard_material.h"
#include "cmx_hud_material.h"
#include "cmx_mesh_material.h"
#include "cmx_model.h"
#include "cmx_pipeline.h"
#include "cmx_primitives.h"
#include "cmx_render_system.h"
#include "cmx_scene.h"
#include "cmx_shaded_material.h"
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

    addMaterial(new HudMaterial(), "hud_material");
    addMaterial(new ShadedMaterial(), "shaded_material");
    addMaterial(new MeshMaterial(), "mesh_material");
    addMaterial(new BillboardMaterial(), "billboard_material");
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

    {
        auto it = _materials.begin();

        while (it != _materials.end())
        {
            (*it).second->free();
            spdlog::info("AssetsManager: Unloaded material [{0}]", (*it).first);
            it++;
        }

        _materials.clear();
    }
    spdlog::info("AssetsManager: Successfully unloaded assets manager!");
}

void AssetsManager::editor()
{
    ImGui::Button("testing");
}

void AssetsManager::addMaterial(Material *material, const std::string &name)
{
    if (material == nullptr)
        return;

    material->name = name;
    material->initialize();

    if (_materials.find(name) != _materials.end())
    {
        _materials[name]->free();
        delete _materials[name];
    }

    _materials[name] = material;
}

Material *AssetsManager::getMaterial(const std::string &name)
{
    if (_materials.find(name) == _materials.end())
    {
        spdlog::warn("AssetsManager: No such material named '{0}'", name);
        return nullptr;
    }

    return _materials[name];
}

void AssetsManager::addModel(const std::string &filepath, const std::string &name)
{
    if (_models.find(name) != _models.end())
    {
        spdlog::warn("AssetsManager: model of same name '{0}' already exists", name);
    }
    else
    {
        Device *device = RenderSystem::getInstance()->getDevice();
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

Model *AssetsManager::getModel(const std::string &name)
{
    if (_models.find(name) == _models.end())
    {
        spdlog::warn("AssetsManager: No such model named '{0}'", name);
        return nullptr;
    }

    return _models[name].get();
}

void AssetsManager::addTexture(const std::string &filepath, const std::string &name)
{
    if (_textures.find(name) != _textures.end())
    {
        spdlog::warn("AssetsManager: texture of same name '{0}' already exists", name);
    }
    else
    {
        Device *device = RenderSystem::getInstance()->getDevice();
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
