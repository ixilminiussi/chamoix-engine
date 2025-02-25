#ifndef CMX_GRAPHICS_MANAGER
#define CMX_GRAPHICS_MANAGER

// std
#include "cmx/cmx_light_environment.h"
#include <map>
#include <memory>
#include <vector>

namespace cmx
{

class GraphicsManager
{
  public:
    GraphicsManager(std::map<uint8_t, std::shared_ptr<class RenderSystem>> &);

    void addToQueue(std::shared_ptr<class Component>);
    void removeFromQueue(std::shared_ptr<class Component>);

    void drawComponents(std::weak_ptr<class Camera>, const class LightEnvironment *);

    void editor();

  private:
    std::map<uint8_t, std::shared_ptr<class RenderSystem>> &_renderSystems;
    std::map<uint8_t, std::vector<std::shared_ptr<class Component>>> _componentRenderQueue{};

    // warning flags
    bool _noCameraFlag{false};
};

} // namespace cmx

#endif
