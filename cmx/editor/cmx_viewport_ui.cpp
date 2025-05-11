#include "cmx_sink.h"
#include <spdlog/common.h>
#include <spdlog/sinks/ansicolor_sink.h>
#ifndef NDEBUG
#include "cmx_viewport_ui.h"

// cmx
#include "ImGuizmo.h"
#include "cmx_actor.h"
#include "cmx_assets_manager.h"
#include "cmx_component.h"
#include "cmx_descriptors.h"
#include "cmx_editor.h"
#include "cmx_frame_info.h"
#include "cmx_game.h"
#include "cmx_graphics_manager.h"
#include "cmx_input_manager.h"
#include "cmx_register.h"
#include "cmx_render_system.h"
#include "cmx_renderer.h"
#include "cmx_transform.h"
#include "cmx_viewport_actor.h"

// lib
#include <IconsMaterialSymbols.h>
#include <glm/fwd.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <imgui_internal.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>

// std
#include <cstdlib>
#include <memory>

namespace cmx
{

ViewportUI::ViewportUI()
{
    _cmxRegister = &Register::getInstance();
}

ViewportUI::~ViewportUI()
{
    ImGui_ImplVulkan_Shutdown();
}

std::string inspectedName;

void ViewportUI::saveState()
{
    if (_inspectedActor != nullptr)
    {
        inspectedName = _inspectedActor->name;
    }
    else
    {
        inspectedName = "";
    }
}

void ViewportUI::reloadState()
{
    if (inspectedName.compare("") != 0)
    {
        _inspectedActor = _attachedScene->getActorByName(inspectedName);
    }
}

void ViewportUI::initImGUI()
{
    RenderSystem *renderSystem = RenderSystem::getInstance();
    Device &cmxDevice = *renderSystem->getDevice();
    Window &cmxWindow = *renderSystem->_window;

    _imguiPool = DescriptorPool::Builder(cmxDevice)
                     .setMaxSets(1000)
                     .addPoolSize(vk::DescriptorType::eSampler, 1000)
                     .addPoolSize(vk::DescriptorType::eCombinedImageSampler, 1000)
                     .addPoolSize(vk::DescriptorType::eSampledImage, 1000)
                     .addPoolSize(vk::DescriptorType::eStorageImage, 1000)
                     .addPoolSize(vk::DescriptorType::eUniformTexelBuffer, 1000)
                     .addPoolSize(vk::DescriptorType::eStorageTexelBuffer, 1000)
                     .addPoolSize(vk::DescriptorType::eUniformBuffer, 1000)
                     .addPoolSize(vk::DescriptorType::eStorageBuffer, 1000)
                     .addPoolSize(vk::DescriptorType::eUniformBufferDynamic, 1000)
                     .addPoolSize(vk::DescriptorType::eStorageBufferDynamic, 1000)
                     .addPoolSize(vk::DescriptorType::eInputAttachment, 1000)
                     .build();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplGlfw_InitForVulkan(cmxWindow.getGLFWwindow(), true);

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = cmxDevice.instance();
    init_info.PhysicalDevice = cmxDevice.physicalDevice();
    init_info.Device = cmxDevice.device();
    init_info.Queue = cmxDevice.graphicsQueue();
    init_info.DescriptorPool = _imguiPool->getDescriptorPool().operator VkDescriptorPool();
    init_info.MinImageCount = 3;
    init_info.ImageCount = 3;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.RenderPass = renderSystem->_renderer->getSwapChainRenderPass().operator VkRenderPass();

    ImGui_ImplVulkan_Init(&init_info);

    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF((std::string(EDITOR_FILES) + std::string("JetBrainsMonoNL-Regular.ttf")).c_str(),
                                 16.0f);
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    float iconFontSize = 16.0f;

    static const ImWchar icons_ranges[] = {ICON_MIN_MS, ICON_MAX_16_MS, 0};
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.GlyphOffset = {0.0f, 4.0f};
    io.Fonts->AddFontFromFileTTF((std::string(EDITOR_FILES) + std::string("MaterialIcons-Regular.ttf")).c_str(),
                                 iconFontSize, &icons_config, icons_ranges);

    _saveFileDialog = ImGui::FileBrowser(ImGuiFileBrowserFlags_EnterNewFilename | ImGuiFileBrowserFlags_CreateNewDir);
    _openFileDialog = ImGui::FileBrowser(ImGuiFileBrowserFlags_CreateNewDir);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
}

void ViewportUI::render(const struct FrameInfo &frameInfo)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();

    renderDockSpace();

    renderTopBar();
    renderPlayBar();
    renderCurrentSceneMetaData();

    if (_showScene)
        renderScene();

    if (_showGraphicsManager)
        renderGraphicsManager();

    if (_showProjectSettings)
        renderProjectSettings();

    if (_showLogger)
        renderLogger();

    if (_showViewportSettings)
        renderViewportSettings();

    if (_showSceneTree)
        renderSceneTree();

    if (_showAssetsManager)
        renderAssetsManager();

    if (_showWorldManager)
        renderWorldManager();

    if (_showInspector)
        renderInspector();

    renderPlayButton();

    _saveFileDialog.Display();
    _openFileDialog.Display();

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), frameInfo.commandBuffer);
}

void ViewportUI::update()
{
    if (_saveFileDialog.HasSelected())
    {
        _attachedScene->saveAs(_saveFileDialog.GetSelected().string().c_str());
        _saveFileDialog.ClearSelected();
    }

    if (_openFileDialog.HasSelected())
    {
        _attachedScene->unload();
        _attachedScene->getGame()->getInputManager()->unbindAll();

        if (Editor *editor = Editor::getInstance())
        {
            _attachedScene->loadFrom(_openFileDialog.GetSelected().string().c_str());

            if (ViewportActor *viewportActor = editor->getViewportActor())
            {
                _attachedScene->setCamera(viewportActor->getCamera(), true);
            }
        }

        _openFileDialog.ClearSelected();
    }
}

void ViewportUI::renderDockSpace()
{
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar |
                                    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::SetNextWindowPos(ImVec2(0, _playBarHeight));
    ImGui::Begin("DockSpace Window", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("GlobalDockspace");
    ImGui::DockSpace(dockspace_id, ImVec2(0, 0));

    ImGui::End();

    static bool doneAlready{false};

    if (!doneAlready)
    {
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

        ImGuiID dock_main_id = dockspace_id;
        ImGuiID left_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.2f, nullptr, &dock_main_id);
        ImGuiID left_bottom_id = ImGui::DockBuilderSplitNode(left_id, ImGuiDir_Down, 0.2f, nullptr, &left_id);
        ImGuiID right_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.2f, nullptr, &dock_main_id);
        ImGuiID bottom_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.3f, nullptr, &dock_main_id);

        ImGui::DockBuilderDockWindow("Scene Tree", left_id);
        ImGui::DockBuilderDockWindow("Graphics Manager", left_bottom_id);
        ImGui::DockBuilderDockWindow("World Manager", left_bottom_id);
        ImGui::DockBuilderDockWindow("Inspector", right_id);
        ImGui::DockBuilderDockWindow("Logger", bottom_id);
        ImGui::DockBuilderDockWindow("Scene", dock_main_id);

        ImGui::DockBuilderFinish(dockspace_id);

        doneAlready = true;
    }

    _centralNode = ImGui::DockBuilderGetCentralNode(dockspace_id);
}

void ViewportUI::renderTopBar()
{
    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem(ICON_MS_SAVE " Save", "Ctrl+S"))
        {
            if (_attachedScene == nullptr)
            {
                spdlog::warn("ViewportUI: No attached scene!");
            }
            else
            {
                _attachedScene->save();
            }
        }
        if (ImGui::MenuItem(ICON_MS_SAVE " Save as", "Ctrl+S"))
        {
            if (_attachedScene == nullptr)
            {
                spdlog::warn("ViewportUI: No attached scene!");
            }
            else
            {
                _saveFileDialog.SetPwd(GAME_FILES);
                _saveFileDialog.SetTitle("choose save location");
                _saveFileDialog.SetTypeFilters({".xml"});
                _saveFileDialog.Open();
            }
        }
        if (ImGui::MenuItem(ICON_MS_UPLOAD_FILE " Load from", "Ctrl+S"))
        {
            _openFileDialog.SetPwd(GAME_FILES);
            _openFileDialog.SetTitle("choose save location");
            _openFileDialog.SetTypeFilters({".xml"});
            _openFileDialog.Open();
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Settings"))
    {
        if (ImGui::MenuItem(ICON_MS_SETTINGS " Viewport Settings"))
        {
            renderViewportSettings();
        }
        if (ImGui::MenuItem(ICON_MS_SETTINGS " Project Settings"))
        {
            renderProjectSettings();
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Toolbar"))
    {
        if (ImGui::MenuItem(ICON_MS_FOUNDATION " Scene Tree", "Ctrl+T"))
        {
            renderSceneTree();
        }
        if (ImGui::MenuItem(ICON_MS_ALBUM " Assets", "Ctrl+A"))
        {
            renderAssetsManager();
        }
        if (ImGui::MenuItem(ICON_MS_EYE_TRACKING " Graphics Manager"))
        {
            renderGraphicsManager();
        }
        if (ImGui::MenuItem(ICON_MS_SETTINGS " Scene Manager"))
        {
            renderWorldManager();
        }
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();

    _menuBarHeight = ImGui::GetFrameHeight();
}

void ViewportUI::renderScene()
{
    _showScene = true;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Scene");
    ImGui::PopStyleVar(3);

    ImVec2 origin = ImGui::GetCursorScreenPos();
    ImGui::SetNextWindowPos(ImVec2(origin.x + 20, origin.y + 20));
    renderGuizmoManager();

    RenderSystem *renderSystem = RenderSystem::getInstance();
    if (renderSystem)
    {
        VkDescriptorSet descriptorSet =
            renderSystem->getSamplerDescriptorSet(renderSystem->getSamplerDescriptorSetID()).operator VkDescriptorSet();

        _sceneViewportSize = ImGui::GetContentRegionAvail();
        ImGui::BeginChild("SceneViewport", _sceneViewportSize, false);
        ImGui::Image((ImTextureID)descriptorSet, ImVec2(_sceneViewportSize.x, _sceneViewportSize.y));
        _isHoveringSceneViewport = ImGui::IsItemHovered();

        if (_inspectedActor)
        {
            ImGuizmo::BeginFrame();
            ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());

            ImVec2 size = ImGui::GetWindowSize();
            ImGuizmo::SetRect(origin.x, origin.y, size.x, size.y);
            _inspectedActor->Transformable::editor(Editor::getInstance()->getViewportActor()->getCamera().get());
        }

        ImGui::EndChild();
    }

    ImGui::End();
}

ImVec2 ViewportUI::getSceneViewportSize()
{
    return _sceneViewportSize;
}

void ViewportUI::renderPlayBar()
{
    ImGui::SetNextWindowPos(ImVec2(0, _menuBarHeight));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, _playBarHeight));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoCollapse;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("Toolbar", nullptr, flags);

    float windowWidth = ImGui::GetContentRegionAvail().x;
    float buttonWidth = 40.0f;

    ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
    if (ImGui::Button(ICON_MS_PLAY_ARROW "##PlayBar", ImVec2(buttonWidth, 0)))
    {
        Editor *editor = Editor::getInstance();
        editor->declarePlayIntent();
    }

    ImGui::End();
    ImGui::PopStyleVar(2);
}

void ViewportUI::renderViewportSettings()
{
    _showViewportSettings = true;
    ImGui::Begin("Viewport Settings", &_showViewportSettings, ImGuiWindowFlags_AlwaysAutoResize);

    if (Editor *editor = Editor::getInstance())
    {
        if (ViewportActor *viewportActor = editor->getViewportActor())
        {
            viewportActor->editor();
        }
        ImGui::SeparatorText("Shortcuts");
        if (InputManager *inputManager = editor->getInputManager())
        {
            inputManager->editor();
        }
    }

    ImGui::End();
}

void ViewportUI::renderProjectSettings()
{
    if (_attachedScene == nullptr)
    {
        spdlog::warn("ViewportUI: No attached scene!");
    }

    static int activeTab = 0;

    Game *game = _attachedScene->getGame();

    _showProjectSettings = true;
    ImGui::Begin("Project Settings", &_showProjectSettings, ImGuiWindowFlags_AlwaysAutoResize);

    if (ImGui::BeginTabBar(""))
    {
        if (ImGui::BeginTabItem("Input Manager"))
        {
            activeTab = 0;

            game->getInputManager()->editor();

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();

        ImGui::End();
    }
}

void ViewportUI::renderWorldManager()
{
    if (_attachedScene == nullptr)
    {
        spdlog::warn("ViewportUI: No attached scene!");
    }

    static int activeTab = 0;

    _showWorldManager = true;

    ImGui::Begin("World Manager", &_showWorldManager, ImGuiWindowFlags_AlwaysAutoResize);

    if (ImGui::BeginTabBar(""))
    {
        if (ImGui::BeginTabItem("Light Environmnent"))
        {
            activeTab = 0;

            _attachedScene->getLightEnvironment()->editor();

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
    ImGui::End();
}

void ViewportUI::renderSceneTree()
{
    static Actor *editing{nullptr};

    if (_attachedScene == nullptr)
    {
        spdlog::warn("ViewportUI: No attached scene!");
    }

    _showSceneTree = true;
    ImGui::Begin("Scene Tree", &_showSceneTree, ImGuiWindowFlags_AlwaysAutoResize);

    std::vector<Actor *> actorList{};
    _attachedScene->getAllActorsByType<Actor>(actorList);

    std::sort(actorList.begin(), actorList.end(), [](const Actor *a, const Actor *b) { return (a->name < b->name); });

    auto it = actorList.begin();
    int i = 0;

    // render existing actors
    while (it != actorList.end())
    {
        ImGui::PushID(i++);
        Actor *actor = *it;
        if (actor)
        {
            if (editing == actor)
            {
                static char buffer[100];
                if (ImGui::Button(ICON_MS_CHECK))
                {
                    if (_attachedScene->renameActor(actor, std::string(buffer)))
                        editing = nullptr;
                }
                ImGui::SameLine();
                ImGui::SetNextItemWidth(135);
                if (ImGui::InputText("##", buffer, 100, ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    if (_attachedScene->renameActor(actor, std::string(buffer)))
                        editing = nullptr;
                }
            }
            else
            {
                if (ImGui::Button(ICON_MS_EDIT))
                {
                    editing = actor;
                }
                ImGui::SameLine();
                if (ImGui::Button(actor->name.c_str()))
                {
                    _inspectedActor = *it;
                    renderInspector();
                }
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_MS_DELETE))
            {
                _attachedScene->removeActor(actor);
                ImGui::PopID();
                _inspectedActor = nullptr;
                continue;
            }
        }

        ImGui::PopID();
        it++;
    }

    // create new actor
    static const char *selected = _cmxRegister->getActorRegister().begin()->first.c_str();

    ImGui::SeparatorText("New Actor");
    ImGui::PushID(i++);
    ImGui::SetNextItemWidth(170);
    if (ImGui::BeginCombo("##", selected))
    {
        for (const auto &[name, func] : _cmxRegister->getActorRegister())
        {
            bool isSelected = (strcmp(selected, name.c_str()) == 0);

            if (ImGui::Selectable(name.c_str(), isSelected))
            {
                selected = name.c_str();
            }

            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }
    ImGui::PopID();

    ImGui::SameLine();
    if (ImGui::Button(ICON_MS_ADD))
    {
        std::string name = std::string(selected);
        _cmxRegister->spawnActor(selected, _attachedScene, name.c_str());
    }

    ImGui::End();
}

void ViewportUI::renderInspector()
{
    _showInspector = true;
    ImGui::Begin("Inspector", &_showInspector, ImGuiWindowFlags_AlwaysAutoResize);

    if (_inspectedActor != nullptr)
    {
        ImGui::Text("%s", _inspectedActor->name.c_str());
        ImGui::Separator();
        _inspectedActor->editor();
    }
    else
    {
        ImGui::Text("No actor selected.");
    }

    ImGui::End();
}

void ViewportUI::renderGraphicsManager()
{
    _showGraphicsManager = true;
    ImGui::Begin("Graphics Manager", &_showGraphicsManager, ImGuiWindowFlags_AlwaysAutoResize);

    _attachedScene->getGraphicsManager()->editor(_attachedScene->getAssetsManager());

    ImGui::End();
}

void ViewportUI::renderGuizmoManager()
{
    ImGui::Begin("##2", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);

    if (ImGui::RadioButton(ICON_MS_OPEN_WITH, Transformable::currentGuizmoOperation == ImGuizmo::TRANSLATE))
        guizmoToTranslate(0.f, 0);

    ImGui::SameLine();
    if (ImGui::RadioButton(ICON_MS_ROTATE_RIGHT, Transformable::currentGuizmoOperation == ImGuizmo::ROTATE))
        guizmoToRotate(0.f, 0);

    ImGui::SameLine();
    if (ImGui::RadioButton(ICON_MS_ZOOM_OUT_MAP, Transformable::currentGuizmoOperation == ImGuizmo::SCALE))
        guizmoToScale(0.f, 0);

    ImGui::SameLine();
    ImGui::Checkbox("snap", &Transformable::guizmoSnap);
    if (Transformable::guizmoSnap)
    {
        ImGui::SameLine();
        ImGui::SetNextItemWidth(30.f);
        ImGui::DragFloat("snap to", &Transformable::guizmoSnapTo, 0.01f, 0.01f, 10.f);
    }

    ImGui::End();
}

void ViewportUI::renderAssetsManager()
{
    _showAssetsManager = true;

    ImGui::Begin("Assets Manager", &_showInspector);
    if (AssetsManager *assetsManager = _attachedScene->getAssetsManager())
    {
        assetsManager->editor();
    }
    ImGui::End();
}

void ViewportUI::renderLogger()
{
    static bool bOneTime{true};
    static std::shared_ptr<EditorSink<std::mutex>> editorSink = std::make_shared<EditorSink<std::mutex>>();
    if (bOneTime)
    {
        auto terminalSink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
        std::vector<spdlog::sink_ptr> sinks{terminalSink, editorSink};

        std::shared_ptr<spdlog::logger> editorLogger = std::make_shared<spdlog::logger>("", sinks.begin(), sinks.end());
        spdlog::set_default_logger(editorLogger);
        bOneTime = false;
    }

    _showLogger = true;

    if (ImGui::Begin("Logger"))
    {
        ImGui::BeginChild("LogRegion", ImVec2(0, 0), 0, ImGuiWindowFlags_HorizontalScrollbar);

        for (const auto &[string, logLevel] : editorSink->getLines())
        {
            ImVec4 color;
            switch (logLevel)
            {
            case spdlog::level::level_enum::err:
                color = {255.f, 85.f, 85.f, 255.f};
                break;
            case spdlog::level::level_enum::warn:
                color = {241.f, 250.f, 140.f, 255.f};
                break;
            case spdlog::level::level_enum::critical:
                color = {255.f, 121.f, 198.f, 255.f};
                break;
            case spdlog::level::level_enum::debug:
                color = {139.f, 233.f, 253.f, 255.f};
                break;
            default:
                color = {80.f, 250.f, 123.f, 255.0f};
                break;
            }
            color.x /= 255.f;
            color.y /= 255.f;
            color.z /= 255.f;
            color.w /= 255.f;

            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::TextUnformatted(string.c_str());
            ImGui::PopStyleColor();
        }

        ImGui::EndChild();
        ImGui::End();
    }
}

void ViewportUI::renderPlayButton()
{
    //     ImGuiIO &io = ImGui::GetIO();
    //
    //     ImVec2 centerPos = ImVec2(io.DisplaySize.x - 300.0f, 30.0f);
    //
    //     ImGui::SetNextWindowPos(centerPos, ImGuiCond_Always);
    //
    //     ImGui::Begin("##", nullptr,
    //                  ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
    //                      ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);
    //
    //     if (ImGui::Button(ICON_MS_PLAY_ARROW "F9"))
    //     {
    //         Editor *editor = Editor::getInstance();
    //         editor->toggle(0.f, 0);
    //     }
    //
    //     ImGui::End();
}

void ViewportUI::renderCurrentSceneMetaData()
{
    if (_centralNode != nullptr)
    {
        ImVec2 pos = _centralNode->Pos;
        ImVec2 size = _centralNode->Size;
        pos.x += 20.f;
        pos.y += size.y - 40.f;
        ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
    }

    ImGui::Begin("##3", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoNavFocus |
                     ImGuiWindowFlags_NoBackground);

    ImGui::Text("%s", _attachedScene->getXMLPath().c_str());

    ImGui::End();
}

void ViewportUI::guizmoToRotate(float, int)
{
    Transformable::currentGuizmoOperation = ImGuizmo::ROTATE;
}

void ViewportUI::guizmoToScale(float, int)
{
    Transformable::currentGuizmoOperation = ImGuizmo::SCALE;
}

void ViewportUI::guizmoToTranslate(float, int)
{
    Transformable::currentGuizmoOperation = ImGuizmo::TRANSLATE;
}

void ViewportUI::duplicateSelected(float, int)
{
    if (_inspectedActor != nullptr)
    {
        _inspectedActor = Actor::duplicate(_attachedScene, _inspectedActor);
    }
}

} // namespace cmx
#endif
