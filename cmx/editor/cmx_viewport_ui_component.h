#pragma once

#include "cmx_component.h"
#include <memory>

namespace cmx
{

class ViewportUIComponent : public Component
{
  public:
    ViewportUIComponent(float &vpMoveSpeed, float &vpSensitivity);
    ~ViewportUIComponent();

    void update(float dt) override;
    void render(class FrameInfo &, VkPipelineLayout) override;

    // TODO: very own input manager logic
    void load();
    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentComponent) override;

    void renderTopBar();
    void renderViewportSettings();
    void renderProjectSettings();
    void renderSceneTree();
    void renderInspector();

    void initImGUI(class RenderSystem *);
    void initInputManager(class CmxWindow &, const std::string &shortcutsPath = "editor/shortcuts.xml");

    class InputManager *getInputManager()
    {
        return inputManager.get();
    }

  private:
    bool initialized{false};
    std::unique_ptr<class CmxDescriptorPool> imguiPool;

    std::unique_ptr<class InputManager> inputManager;
    bool showViewportSettings{false};
    bool showProjectSettings{false};
    bool showSceneTree{true};
    bool showInspector{false};
    std::weak_ptr<Actor> inspectedActor;

    // viewport actor settings
    float &viewportMovementSpeed;
    float &viewportSensitivity;
};

} // namespace cmx
