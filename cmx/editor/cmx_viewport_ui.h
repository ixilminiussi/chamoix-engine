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

    void render(const struct FrameInfo &);
    void update();

    void load();
    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parent);

    void saveState();
    void reloadState();

    void guizmoToRotate(float, int);
    void guizmoToScale(float, int);
    void guizmoToTranslate(float, int);
    void duplicateSelected(float, int);

    void initImGUI();

    void attachScene(class Scene *scene)
    {
        _attachedScene = scene;
    };

    ImVec2 getSceneViewportSize();

    bool isHoveringSceneViewport()
    {
        return _isHoveringSceneViewport;
    }

  private:
    void renderScene();
    void renderDockSpace();
    void renderTopBar();
    void renderPlayBar();
    void renderViewportSettings();
    void renderProjectSettings();
    void renderWorldManager();
    void renderSceneTree();
    void renderInspector();
    void renderGraphicsManager();
    void renderGuizmoManager();
    void renderAssetsManager();
    void renderPlayButton();
    void renderCurrentSceneMetaData();

    ImVec2 _sceneViewportSize;
    bool _isHoveringSceneViewport;

    ImGui::FileBrowser _saveFileDialog;
    ImGui::FileBrowser _openFileDialog;
    ImGuiDockNode *_centralNode{nullptr};

    bool _initialized{false};
    std::unique_ptr<class DescriptorPool> _imguiPool;

    bool _showScene{true};
    bool _showViewportSettings{false};
    bool _showProjectSettings{false};
    bool _showWorldManager{true};
    bool _showSceneTree{true};
    bool _showInspector{true};
    bool _showAssetsManager{false};
    bool _showGraphicsManager{true};
    Actor *_inspectedActor{nullptr};

    class Scene *_attachedScene;
    class Register *_cmxRegister;

    int _menuBarHeight;
    const int _playBarHeight = 40;
};

} // namespace cmx

#endif
