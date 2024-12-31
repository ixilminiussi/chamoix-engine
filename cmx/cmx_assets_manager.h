#ifndef CMX_ASSET_MANAGER
#define CMX_ASSET_MANAGER

// lib
#include "tinyxml2.h"

// std
#include <memory>
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
    AssetsManager(class Scene *parent);
    ~AssetsManager() = default;

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &, tinyxml2::XMLElement *);
    void load(tinyxml2::XMLElement *);
    void unload();

    void editor();

    void addModel(const std::string &filepath, const std::string &name);
    void removeModel(const std::string &name);
    std::shared_ptr<class CmxModel> getModel(const std::string &name);

    const auto &getModels()
    {
        return _models;
    }

  private:
    class Scene *_parentScene;

    // regular pointers, WE are MANAGING the life cycle of these assets
    std::unordered_map<std::string, std::shared_ptr<class CmxModel>> _models;
    // TODO:
    // fonts
    // textures
    // sprites
};

} // namespace cmx

#endif
