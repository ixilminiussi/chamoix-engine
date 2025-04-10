#include "cmx_assets_manager.h"

// cmx
#include "cmx_billboard_material.h"
#include "cmx_dithered_material.h"
#include "cmx_dithered_textured_material.h"
#include "cmx_hud_material.h"
#include "cmx_mesh_material.h"
#include "cmx_model.h"
#include "cmx_primitives.h"
#include "cmx_register.h"
#include "cmx_render_system.h"
#include "cmx_scene.h"
#include "cmx_shaded_material.h"
#include "cmx_texture.h"

// lib
#include "cmx_utils.h"
#include "imgui.h"

// std
#include <cstdlib>
#include <stdexcept>

namespace cmx
{

AssetsManager::AssetsManager(class Scene *parent)
    : _parentScene{parent}, _models{}, _textures2D{}, _textures3D{}, _materials{}
{
    addModel("assets/cmx/cube.obj", PRIMITIVE_CUBE);
    addModel("assets/cmx/cylinder.obj", PRIMITIVE_CYLINDER);
    addModel("assets/cmx/plane.obj", PRIMITIVE_PLANE);
    addModel("assets/cmx/sphere.obj", PRIMITIVE_SPHERE);
    addModel("assets/cmx/torus.obj", PRIMITIVE_TORUS);
    addModel("assets/cmx/camera.obj", "cmx_camera");

    add2DTexture("assets/cmx/missing-texture.png", "cmx_missing");
    add2DTexture("assets/cmx/point-light.png", "cmx_point_light");

    addMaterial(new ShadedMaterial(), "shaded_material");
    addMaterial(new MeshMaterial(), "mesh_material");
    addMaterial(new DitheredMaterial(), "dithered_material");
    addMaterial(new DitheredTexturedMaterial(), "dithered_textured_material");
    addMaterial(new HudMaterial(), "hud_material");
    addMaterial(new BillboardMaterial(), "billboard_material");
};

tinyxml2::XMLElement &AssetsManager::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement)
{
    tinyxml2::XMLElement *assetsElement = doc.NewElement("assets");

    for (const auto &pair : _materials)
    {
        tinyxml2::XMLElement *materialElement = pair.second->save(doc, assetsElement);
        if (materialElement != nullptr)
        {
            materialElement->SetAttribute("name", pair.first.c_str());
        }
    }

    for (const auto &pair : _models)
    {
        tinyxml2::XMLElement &modelElement = pair.second->save(doc, assetsElement);
        modelElement.SetAttribute("name", pair.first.c_str());
    }

    for (const auto &pair : _textures2D)
    {
        tinyxml2::XMLElement &textureElement = pair.second->save(doc, assetsElement);
        textureElement.SetAttribute("name", pair.first.c_str());
    }

    for (const auto &pair : _textures3D)
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
            if (tinyxml2::XMLElement *filepathElement = textureElement->FirstChildElement("layer"))
            {
                std::vector<std::string> filepaths{};
                while (filepathElement)
                {
                    filepaths.push_back(filepathElement->Attribute("filepath"));

                    filepathElement = filepathElement->NextSiblingElement("layer");
                }

                add3DTexture(filepaths, textureElement->Attribute("name"));
            }
            else
            {
                add2DTexture(textureElement->Attribute("filepath"), textureElement->Attribute("name"));
            }

            textureElement = textureElement->NextSiblingElement("texture");
        }

        tinyxml2::XMLElement *materialElement = assetsElement->FirstChildElement("material");
        while (materialElement)
        {
            Material *material = Register::getInstance().getMaterial(materialElement->Attribute("type"));
            const char *name = materialElement->Attribute("name");

            if (!addMaterial(material, materialElement->Attribute("name")))
            {
                getMaterial(materialElement->Attribute("name"))->load(materialElement);
                delete material;
            }
            else
            {
                material->load(materialElement);
            }

            materialElement = materialElement->NextSiblingElement("material");
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
        auto it = _textures2D.begin();

        while (it != _textures2D.end())
        {
            (*it).second->free();
            spdlog::info("AssetsManager: Unloaded 2D texture [{0}]", (*it).first);
            it++;
        }

        _textures2D.clear();
    }

    {
        auto it = _textures3D.begin();

        while (it != _textures3D.end())
        {
            (*it).second->free();
            spdlog::info("AssetsManager: Unloaded 3D texture [{0}]", (*it).first);
            it++;
        }

        _textures3D.clear();
    }

    {
        auto it = _materials.begin();

        while (it != _materials.end())
        {
            (*it).second->free();
            delete (*it).second;
            spdlog::info("AssetsManager: Unloaded material [{0}]", (*it).first);
            it++;
        }

        _materials.clear();
    }
    spdlog::info("AssetsManager: Successfully unloaded assets manager!");
}

void AssetsManager::editor()
{
}

bool AssetsManager::addMaterial(Material *material, const char *name)
{
    if (_materials.find(std::string(name)) != _materials.end())
    {
        spdlog::warn("AssetsManager: material of same name '{0}' already exists", name);
        return false;
    }

    if (material == nullptr)
        return false;

    material->name = name;
    material->initialize();

    if (_materials.find(std::string(name)) != _materials.end())
    {
        _materials[name]->free();
        delete _materials[name];
    }

    _materials[name] = material;

    return true;
}

Material *AssetsManager::makeUnique(const char *name, bool doNotSave)
{
    if (_materials.find(std::string(name)) == _materials.end())
    {
        spdlog::warn("assetsmanager: no such material named '{0}'", name);
        return nullptr;
    }

    Material *duplicate = _materials[name]->clone(doNotSave);
    std::string newName = incrementNumberInParentheses(name);
    while (_materials.find(newName) != _materials.end())
    {
        newName = incrementNumberInParentheses(newName);
    }
    addMaterial(duplicate, newName.c_str());

    return duplicate;
}

Material *AssetsManager::getMaterial(const char *name)
{
    if (_materials.find(std::string(name)) == _materials.end())
    {
        spdlog::warn("assetsmanager: no such material named '{0}'", name);
        return nullptr;
    }

    return _materials[name];
}

void AssetsManager::addModel(const char *filepath, const char *name)
{
    if (_models.find(std::string(name)) != _models.end())
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

void AssetsManager::removeModel(const char *name)
{
    // TODO: implement
}

Model *AssetsManager::getModel(const char *name)
{
    if (_models.find(std::string(name)) == _models.end())
    {
        spdlog::warn("AssetsManager: No such model named '{0}'", name);
        return nullptr;
    }

    return _models[name].get();
}

void AssetsManager::add2DTexture(const char *filepath, const char *name)
{
    if (_textures2D.find(std::string(name)) != _textures2D.end())
    {
        spdlog::warn("AssetsManager: texture of same name '{0}' already exists", name);
    }
    else
    {
        Device *device = RenderSystem::getInstance()->getDevice();
        if (device)
        {
            _textures2D[name] = std::unique_ptr<Texture>(Texture::create2DTextureFromFile(device, filepath, name));
        }
    }
}

void AssetsManager::add3DTexture(const std::vector<std::string> &filepaths, const char *name)
{
    if (filepaths.size() <= 1)
    {
        throw std::runtime_error("AssetsManager: 3d textures MUST have more than one texture");
    }

    if (_textures3D.find(name) != _textures3D.end())
    {
        spdlog::warn("AssetsManager: texture of same name '{0}' already exists", name);
    }
    else
    {
        Device *device = RenderSystem::getInstance()->getDevice();
        if (device)
        {
            _textures3D[name] = std::unique_ptr<Texture>(Texture::create3DTextureFromFile(device, filepaths, name));
        }
    }
}

void AssetsManager::removeTexture(const char *name)
{
    // TODO: implement
}

Texture *AssetsManager::get2DTexture(const char *name)
{
    if (_textures2D.find(std::string(name)) == _textures2D.end())
    {
        spdlog::warn("AssetsManager: No such texture named '{0}'", name);
        return nullptr;
    }

    return _textures2D[name].get();
}

Texture *AssetsManager::get3DTexture(const char *name)
{
    if (_textures3D.find(std::string(name)) == _textures3D.end())
    {
        spdlog::warn("AssetsManager: No such texture named '{0}'", name);
        return nullptr;
    }

    return _textures3D[name].get();
}

Texture *AssetsManager::getAnyTexture(const char *name)
{
    if (_textures2D.find(std::string(name)) == _textures2D.end())
    {
        if (_textures3D.find(std::string(name)) == _textures3D.end())
        {
            spdlog::warn("AssetsManager: No such texture named '{0}'", name);
            return nullptr;
        }

        return _textures3D[name].get();
    }

    return _textures2D[name].get();
}

} // namespace cmx
