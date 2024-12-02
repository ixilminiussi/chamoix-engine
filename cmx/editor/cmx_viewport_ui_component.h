#pragma once

#include "cmx_component.h"

namespace cmx
{

class ViewportUIComponent : public Component
{
  public:
    ViewportUIComponent(float &vpMoveSpeed, float &vpSensitivity);
    ~ViewportUIComponent();

    void update(float dt) override;
    void render(class FrameInfo &, VkPipelineLayout) override;

    void renderTopBar();
    void renderViewportSettings();
    void renderWorldTree();

    void initImGUI(class CmxDevice &, class CmxWindow &, class CmxRenderer &);

    const char *name{"ViewportUIComponent"};

  private:
    bool initialized{false};
    std::unique_ptr<class CmxDescriptorPool> imguiPool;

    bool showViewportSettings{false};
    bool showWorldTree{true};

    // viewport actor settings
    float &viewportMovementSpeed;
    float &viewportSensitivity;
};

} // namespace cmx
