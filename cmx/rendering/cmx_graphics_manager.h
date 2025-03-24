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
    void render(std::weak_ptr<class Camera>, const class LightEnvironment *);

    void editor();

  private:
    void drawShadowMap(const class FrameInfo *, const class LightEnvironment *);
    void drawRenderQueue(const class FrameInfo *, std::weak_ptr<class Camera>, const class LightEnvironment *);

    std::map<uint8_t, std::vector<std::pair<class Drawable *, class DrawOption *>>> _drawableRenderQueue;

    // warning flags
    bool _noCameraFlag{false};
    class RenderSystem *_renderSystem;
};

} // namespace cmx

#endif
