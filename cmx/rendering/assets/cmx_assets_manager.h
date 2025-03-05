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

    bool addMaterial(class Material *material, const char *name);
    class Material *makeUnique(const char *name);
    class Material *getMaterial(const char *name);
    const auto &getMaterials()
    {
        return _materials;
    }

    void addModel(const char *filepath, const char *name);
    void removeModel(const char *name);
    class Model *getModel(const char *name);
    const auto &getModels()
    {
        return _models;
    }

    void addTexture(const char *filepath, const char *name);
    void removeTexture(const char *name);
    class Texture *getTexture(const char *name);
    const auto &getTextures()
    {
        return _textures;
    }

  private:
    class Scene *_parentScene;

    std::unordered_map<std::string, std::unique_ptr<class Model>> _models;
    std::unordered_map<std::string, std::unique_ptr<class Texture>> _textures;
    std::unordered_map<std::string, class Material *> _materials;
    // TODO:
    // fonts
    // sprites
};

} // namespace cmx

#endif
