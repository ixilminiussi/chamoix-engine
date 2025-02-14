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

class ImGuiDockNode;

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

    void guizmoToRotate(float, int);
    void guizmoToScale(float, int);
    void guizmoToTranslate(float, int);
    void duplicateSelected(float, int);

    void initImGUI();

    void attachScene(class Scene *scene)
    {
        _attachedScene = scene;
    };

  private:
    void renderDockSpace();
    void renderTopBar();
    void renderViewportSettings();
    void renderProjectSettings();
    void renderSceneTree();
    void renderInspector();
    void renderGraphicsManager();
    void renderGuizmoManager();
    void renderAssetsManager();
    void renderPlayButton();
    void renderCurrentSceneMetaData();

    void autoSave();

    ImGui::FileBrowser _fileDialog;
    ImGuiDockNode *_centralNode{nullptr};

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
    Actor *_inspectedActor{nullptr};

    class Scene *_attachedScene;
    class Register *_cmxRegister;
};

} // namespace cmx

#endif
