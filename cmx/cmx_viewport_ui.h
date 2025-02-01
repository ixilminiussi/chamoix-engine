#ifndef CMX_VIEWPORT_UI
#define CMX_VIEWPORT_UI

// cmx
#include "cmx_component.h"

// must be before
#include <imgui.h>
// lib
#include <imfilebrowser.h>

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

    void renderDockSpace();
    void setupDockLayout();
    void renderTopBar();
    void renderViewportSettings();
    void renderProjectSettings();
    void renderSceneTree();
    void renderInspector();
    void renderGraphicsManager();
    void renderAssetsManager();
    void renderPlayButton();

    void attachScene(class Scene *scene)
    {
        _attachedScene = scene;
    };

    void initImGUI();

  private:
    ImGui::FileBrowser _fileDialog;

    bool _initialized{false};
    std::unique_ptr<class DescriptorPool> _imguiPool;

    bool _showViewportSettings{false};
    ImGuiID _projectDockID;
    bool _showProjectSettings{false};
    ImGuiID _sceneTreeDockID;
    bool _showSceneTree{true};
    ImGuiID _inspectorDockID;
    bool _showInspector{false};
    ImGuiID _assetsManagerDockID;
    bool _showAssetsManager{true};
    Actor *_inspectedActor;

    class Scene *_attachedScene;
    class Register *_cmxRegister;
};

} // namespace cmx

#endif
