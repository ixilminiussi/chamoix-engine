#pragma once

#include "cmx_component.h"

namespace cmx
{

class ViewportUIComponent : public Component
{
  public:
    ViewportUIComponent();
    ~ViewportUIComponent();

    void update(float dt) override;
    void render(class FrameInfo &, VkPipelineLayout) override;

    void renderTopBar();
    void renderViewportSettings();

    void initImGUI(class CmxDevice &, class CmxWindow &, class CmxRenderer &);

  private:
    bool initialized{false};
    std::unique_ptr<class CmxDescriptorPool> imguiPool;

    bool showViewportSettings{false};
};

} // namespace cmx
