#pragma once

// std
#include <memory>
#include <vector>

namespace cmx
{

class GraphicsManager
{
  public:
    GraphicsManager(std::shared_ptr<class RenderSystem> renderSystem) : _renderSystem{renderSystem} {};

    void addToQueue(std::shared_ptr<class Component>);
    void removeFromQueue(std::shared_ptr<class Component>);

    void drawComponents(std::weak_ptr<class CameraComponent>);

  private:
    std::shared_ptr<class RenderSystem> _renderSystem;
    std::vector<std::shared_ptr<class Component>> _componentRenderQueue{};
};

} // namespace cmx
