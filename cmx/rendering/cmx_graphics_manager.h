#ifndef CMX_GRAPHICS_MANAGER
#define CMX_GRAPHICS_MANAGER

// cmx
#include "cmx_light_environment.h"

// std
#include <map>
#include <memory>
#include <vector>

namespace cmx
{

class GraphicsManager
{
  public:
    GraphicsManager();

    void add(class Drawable *, class DrawOption *);
    void remove(const class Drawable *);
    void remove(const class DrawOption *);
    void update(class Drawable *, class DrawOption *, size_t oldID);

    void drawRenderQueue(std::weak_ptr<class Camera>, class LightEnvironment *);
    static const std::vector<size_t> &getDescriptorSetIDs();

    void editor(class AssetsManager *assetsManager);
    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &, tinyxml2::XMLElement *) const;
    void load(tinyxml2::XMLElement *, class AssetsManager *assetsManager);

  private:
    void addPostProcess(class Material *material);

    std::map<uint8_t, std::vector<std::pair<class Drawable *, class DrawOption *>>> _drawableRenderQueue;

    // warning flags
    bool _noCameraFlag{false};
    class RenderSystem *_renderSystem;
    std::vector<Material *> _postProcessMaterials;

    static std::vector<size_t> _shadowMapDescriptorSetIDs;
};

} // namespace cmx

#endif
