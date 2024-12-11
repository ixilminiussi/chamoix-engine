#pragma once

// cmx
#include "cmx_model.h"

// lib
#include "tinyxml2.h"
#include <spdlog/spdlog.h>

// std
#include <string>
#include <unordered_map>

namespace cmx
{

// usage:
// every scene has an asset manager which loads and unloads assets on change
// TODO: Global assets which are shared between managers
class AssetsManager final
{
  public:
    AssetsManager(class Scene *parent) : parentScene{parent} {};
    ~AssetsManager() = default;

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &, tinyxml2::XMLElement *);
    void load(tinyxml2::XMLElement *);
    void unload();

    void renderSettings();

    void addModel(const std::string &filepath, const std::string &name);
    void removeModel(const std::string &name);
    std::shared_ptr<CmxModel> getModel(const std::string &name);

  private:
    class Scene *parentScene;

    // regular pointers, WE are MANAGING the life cycle of these assets
    std::unordered_map<std::string, std::shared_ptr<CmxModel>> models;
    // TODO:
    // fonts
    // textures
    // sprites
};

} // namespace cmx
