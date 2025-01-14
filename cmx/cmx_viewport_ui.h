#ifndef CMX_VIEWPORT_UI
#define CMX_VIEWPORT_UI

#include "cmx_component.h"
#include <memory>

namespace cmx
{

class ViewportUI
{
  public:
    ViewportUI(std::shared_ptr<class ViewportActor>, std::shared_ptr<class InputManager>);
    ~ViewportUI();

    void render(const class FrameInfo &);

    void load();
    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parent);

    void renderTopBar();
    void renderViewportSettings();
    void renderProjectSettings();
    void renderSceneTree();
    void renderInspector();
    void renderGraphicsManager();

    void attachScene(class Scene *scene)
    {
        _attachedScene = scene;
    };

    void initImGUI();

    class InputManager *getInputManager()
    {
        return _inputManager.get();
    }

  private:
    bool _initialized{false};
    std::unique_ptr<class CmxDescriptorPool> _imguiPool;

    std::shared_ptr<class InputManager> _inputManager;
    bool _showViewportSettings{false};
    bool _showProjectSettings{false};
    bool _showSceneTree{true};
    bool _showInspector{false};
    std::weak_ptr<Actor> _inspectedActor;

    class Scene *_attachedScene;
    class Register *_cmxRegister;

    std::shared_ptr<ViewportActor> _viewportActor;
};

} // namespace cmx

#endif
