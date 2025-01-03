#ifndef CMX_GRAPHICS_MANAGER
#define CMX_GRAPHICS_MANAGER

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace cmx
{

class GraphicsManager
{
  public:
    GraphicsManager(std::unordered_map<uint8_t, std::shared_ptr<class RenderSystem>> &);

    void addToQueue(std::shared_ptr<class Component>);
    void removeFromQueue(std::shared_ptr<class Component>);

    void drawComponents(std::weak_ptr<class Camera>);

  private:
    std::unordered_map<uint8_t, std::shared_ptr<class RenderSystem>> &_renderSystems;
    std::vector<std::shared_ptr<class Component>> _componentRenderQueue{};

    // warning flags
    bool _noCameraFlag{false};
};

} // namespace cmx

#endif
