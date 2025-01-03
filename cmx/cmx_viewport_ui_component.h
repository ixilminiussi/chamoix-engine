#ifndef CMX_VIEWPORT_UI_COMPONENT
#define CMX_VIEWPORT_UI_COMPONENT

#include "cmx_component.h"
#include <memory>

namespace cmx
{

class ViewportUIComponent : public Component
{
  public:
    ViewportUIComponent();
    ~ViewportUIComponent();

    void update(float dt) override;
    void render(class FrameInfo &, VkPipelineLayout) override;

    void load();
    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentComponent) override;

    void renderTopBar();
    void renderViewportSettings();
    void renderProjectSettings();
    void renderSceneTree();
    void renderInspector();

    void initImGUI();
    void initInputManager(class CmxWindow &, const std::string &shortcutsPath = "editor/shortcuts.xml");

    class InputManager *getInputManager()
    {
        return _inputManager.get();
    }

  private:
    bool _initialized{false};
    std::unique_ptr<class CmxDescriptorPool> _imguiPool;

    std::unique_ptr<class InputManager> _inputManager;
    bool _showViewportSettings{false};
    bool _showProjectSettings{false};
    bool _showSceneTree{true};
    bool _showInspector{false};
    std::weak_ptr<Actor> _inspectedActor;

    class Register *_cmxRegister;
};

} // namespace cmx

#endif
