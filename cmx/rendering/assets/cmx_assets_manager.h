#ifndef CMX_ASSET_MANAGER
#define CMX_ASSET_MANAGER

// lib
#include <tinyxml2.h>

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
    ~AssetsManager();

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &, tinyxml2::XMLElement *);
    void load(tinyxml2::XMLElement *);
    void unload();

    void editor();

    void addModel(const std::string &filepath, const std::string &name);
    void removeModel(const std::string &name);
    class Model *getModel(const std::string &name);

    void addTexture(const std::string &filepath, const std::string &name);
    void removeTexture(const std::string &name);
    class Texture *getTexture(const std::string &name);

    const auto &getModels()
    {
        return _models;
    }

    const auto &getTextures()
    {
        return _textures;
    }

  private:
    class Scene *_parentScene;

    std::unordered_map<std::string, std::unique_ptr<class Model>> _models;
    std::unordered_map<std::string, std::unique_ptr<class Texture>> _textures;
    std::unordered_map<std::string, std::unique_ptr<class Material>> _materials;
    // TODO:
    // fonts
    // sprites
};

} // namespace cmx

#endif
