#include "cmx_assets_manager.h"

// cmx
#include "cmx_game.h"
#include "cmx_model.h"
#include "cmx_scene.h"
#include <cstdlib>
#include <stdexcept>

namespace cmx
{

tinyxml2::XMLElement &AssetsManager::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement)
{
    tinyxml2::XMLElement *assetsElement = doc.NewElement("assets");

    for (const auto &pair : _models)
    {
        tinyxml2::XMLElement &modelElement = pair.second->save(doc, assetsElement);
        modelElement.SetAttribute("name", pair.first.c_str());
    }

    parentElement->InsertEndChild(assetsElement);

    spdlog::info("Saved asset manager");

    return *assetsElement;
}

void AssetsManager::load(tinyxml2::XMLElement *parentElement)
{
    if (tinyxml2::XMLElement *assetsElement = parentElement->FirstChildElement("assets"))
    {
        tinyxml2::XMLElement *modelElement = assetsElement->FirstChildElement("model");
        while (modelElement)
        {
            _models[modelElement->Attribute("name")] =
                CmxModel::createModelFromFile(_parentScene->getGame()->getDevice(), modelElement->Attribute("filepath"),
                                              modelElement->Attribute("name"));

            modelElement = modelElement->NextSiblingElement("model");
        }
    }
}

void AssetsManager::unload()
{
}

void AssetsManager::renderSettings()
{
}

void AssetsManager::addModel(const std::string &filepath, const std::string &name)
{
    try
    {
        _models.at(name);
    }
    catch (const std::out_of_range e)
    {
        _models[name] = CmxModel::createModelFromFile(_parentScene->getGame()->getDevice(), filepath, name);
    }

    spdlog::error("AssetsManager: model of same name '{0}' already exists", name);
}

void AssetsManager::removeModel(const std::string &name)
{
}

std::shared_ptr<CmxModel> AssetsManager::getModel(const std::string &name)
{
    try
    {
        return _models.at(name);
    }
    catch (const std::out_of_range &e)
    {
        spdlog::critical("AssetsManager: No such model named '{0}'", name);
        std::exit(EXIT_FAILURE);
    }
}

} // namespace cmx
