#pragma once

// std
#include <memory>
#include <vector>

namespace cmx
{

class GraphicsManager
{
  public:
    GraphicsManager(std::shared_ptr<class RenderSystem> renderSystem) : renderSystem{renderSystem} {};

    void addToQueue(std::shared_ptr<class Component>);
    void removeFromQueue(std::shared_ptr<class Component>);

    void drawComponents(std::weak_ptr<class CameraComponent>);

  private:
    std::shared_ptr<class RenderSystem> renderSystem;
    std::vector<std::shared_ptr<class Component>> componentRenderQueue{};
};

} // namespace cmx
