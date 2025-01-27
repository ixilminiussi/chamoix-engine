#ifndef CMX_VIEWPORT_UI
#define CMX_VIEWPORT_UI

// cmx
#include "cmx_component.h"

// std
#include <memory>

namespace cmx
{

class ViewportUI
{
  public:
    ViewportUI();
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
    void renderPlayButton();

    void attachScene(class Scene *scene)
    {
        _attachedScene = scene;
    };

    void initImGUI();

  private:
    bool _initialized{false};
    std::unique_ptr<class DescriptorPool> _imguiPool;

    bool _showViewportSettings{false};
    bool _showProjectSettings{false};
    bool _showSceneTree{true};
    bool _showInspector{false};
    Actor *_inspectedActor;

    class Scene *_attachedScene;
    class Register *_cmxRegister;
};

} // namespace cmx

#endif
