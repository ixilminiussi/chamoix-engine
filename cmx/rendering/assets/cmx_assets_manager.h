#ifndef CMX_ASSET_MANAGER
#define CMX_ASSET_MANAGER

// lib
#include <map>
#include <tinyxml2.h>

// std
#include <memory>
#include <string>
#include <vector>

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
    void loadModels(tinyxml2::XMLElement *);
    void loadTextures(tinyxml2::XMLElement *);
    void loadMaterials(tinyxml2::XMLElement *);
    void loadPostProcesses(tinyxml2::XMLElement *);
    void unload();
    void unloadModels();
    void unloadTextures();
    void unloadMaterials();
    void unloadPostProcesses();

    void editor();

    bool addMaterial(class Material *material, const char *name);
    class Material *makeUnique(const char *name, bool doNotSave = true);
    class Material *getMaterial(const char *name);
    const auto &getMaterials()
    {
        return _materials;
    }

    bool addPostProcess(class Material *material, const char *name);
    class Material *getPostProcess(const char *name);
    const auto &getPostProcesses()
    {
        return _postProcesses;
    }

    void addModel(const char *filepath, const char *name);
    void removeModel(const char *name);
    class Model *getModel(const char *name);
    const auto &getModels()
    {
        return _models;
    }

    void add2DTexture(const char *filepath, const char *name);
    void add3DTexture(const std::vector<std::string> &filepaths, const char *name);
    void removeTexture(const char *name);

    class Texture *get2DTexture(const char *name);
    class Texture *get3DTexture(const char *name);
    class Texture *getAnyTexture(const char *name);

    const auto &get3DTextures()
    {
        return _textures3D;
    }

    const auto &get2DTextures()
    {
        return _textures2D;
    }

  private:
    class Scene *_parentScene;

    std::map<std::string, std::unique_ptr<class Model>> _models;
    std::map<std::string, std::unique_ptr<class Texture>> _textures2D;
    std::map<std::string, std::unique_ptr<class Texture>> _textures3D;
    std::map<std::string, class Material *> _materials;
    std::map<std::string, class Material *> _postProcesses;

    // TODO:
    // fonts
    // sprites
};

} // namespace cmx

#endif
