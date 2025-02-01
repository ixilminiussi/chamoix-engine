#include "cmx_viewport_ui.h"

// cmx
#include "cmx_actor.h"
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
#include "cmx_viewport_actor.h"

// lib
#include "IconsMaterialSymbols.h"
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
    _cmxRegister = Register::getInstance();
}

ViewportUI::~ViewportUI()
{
    ImGui_ImplVulkan_Shutdown();
}

void ViewportUI::initImGUI()
{
    // 1: create descriptor pool for IMGUI
    // the size of the pool is very oversize, but it's copied from imgui demo itself.
    Device &cmxDevice = *RenderSystem::getDevice();
    Window &cmxWindow = *RenderSystem::_window;

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
    // 2: initialize imgui library

    // this initializes the core structures of imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // this initializes imgui for SDL
    ImGui_ImplGlfw_InitForVulkan(cmxWindow.getGLFWwindow(), true);

    // this initializes imgui for Vulkan
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = cmxDevice.instance();
    init_info.PhysicalDevice = cmxDevice.physicalDevice();
    init_info.Device = cmxDevice.device();
    init_info.Queue = cmxDevice.graphicsQueue();
    init_info.DescriptorPool = _imguiPool->getDescriptorPool();
    init_info.MinImageCount = 3;
    init_info.ImageCount = 3;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.RenderPass = RenderSystem::_renderer->getSwapChainRenderPass();

    ImGui_ImplVulkan_Init(&init_info);

    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("editor/JetBrainsMonoNL-Regular.ttf", 16.0f);
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    float iconFontSize = 16.0f;

    // merge in icons from Font Awesome
    static const ImWchar icons_ranges[] = {ICON_MIN_MS, ICON_MAX_16_MS, 0};
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.GlyphOffset = {0.0f, 4.0f};
    io.Fonts->AddFontFromFileTTF("editor/MaterialIcons-Regular.ttf", iconFontSize, &icons_config, icons_ranges);
    // use FONT_ICON_FILE_NAME_FAR if you want regular instead of solid
}

void ViewportUI::render(const class FrameInfo &frameInfo)
{
    // imgui new frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();

    renderDockSpace();
    setupDockLayout();

    renderTopBar();

    if (_showProjectSettings)
        renderProjectSettings();

    if (_showViewportSettings)
        renderViewportSettings();

    if (_showSceneTree)
        renderSceneTree();

    if (_showInspector)
        renderInspector();

    if (_showAssetsManager)
        renderAssetsManager();

    renderPlayButton();
    renderGraphicsManager();

    _fileDialog.Display();

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), frameInfo.commandBuffer);
}

void ViewportUI::renderDockSpace()
{
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
                                    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

    ImGui::Begin("DockSpace Window", nullptr, window_flags);
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(1);

    ImGuiID dockspace_id = ImGui::GetID("GlobalDockspace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f),
                     ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();
}

void ViewportUI::setupDockLayout()
{
    ImGuiID dockspace_id = ImGui::GetID("GlobalDockspace");

    ImGui::DockSpaceOverViewport(dockspace_id, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

    // Remove old layout and create a new dockspace
    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

    // Finalize layout
    ImGui::DockBuilderFinish(dockspace_id);
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
                _fileDialog.SetTitle("choose save location");
                _fileDialog.SetTypeFilters({".xml"});
                _fileDialog.Open();
            }
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Settings"))
    {
        if (ImGui::MenuItem(ICON_MS_SETTINGS " Viewport Settings", "Ctrl+V"))
        {
            renderViewportSettings();
        }
        if (ImGui::MenuItem(ICON_MS_SETTINGS " Project Settings", "Ctrl+V"))
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
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
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

void ViewportUI::renderSceneTree()
{
    if (_attachedScene == nullptr)
    {
        spdlog::warn("ViewportUI: No attached scene!");
    }

    _showSceneTree = true;
    ImGui::Begin("Scene Tree", &_showSceneTree, ImGuiWindowFlags_AlwaysAutoResize);

    std::vector<Actor *> actorList{};
    _attachedScene->getAllActorsByType<Actor>(actorList);

    auto it = actorList.begin();
    int i = 0;

    // render existing actors
    while (it != actorList.end())
    {
        ImGui::PushID(i++);
        Actor *actor = *it;
        if (actor)
        {
            if (ImGui::Button(actor->name.c_str()))
            {
                _inspectedActor = *it;
                renderInspector();
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_MS_DELETE))
            {
                _attachedScene->removeActor(actor);
                ImGui::PopID();
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
    ImGui::SetNextItemWidth(203);
    if (ImGui::BeginCombo("##", selected))
    {
        for (const auto &pair : _cmxRegister->getActorRegister())
        {
            bool isSelected = (strcmp(selected, pair.first.c_str()) == 0);

            if (ImGui::Selectable(pair.first.c_str(), isSelected))
            {
                selected = pair.first.c_str();
            }

            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }
    ImGui::PopID();

    static char buffer[100];
    ImGui::SetNextItemWidth(170);
    ImGui::InputText("##", buffer, 100);
    ImGui::SameLine();
    if (ImGui::Button(ICON_MS_ADD))
    {
        _cmxRegister->spawnActor(selected, _attachedScene, buffer);
    }

    ImGui::End();
}

void ViewportUI::renderInspector()
{
    _showInspector = true;
    ImGui::Begin("Inspector", &_showInspector, ImGuiWindowFlags_AlwaysAutoResize);

    Actor *actor = _inspectedActor;
    if (actor)
    {
        ImGui::Text("%s", actor->name.c_str());
        ImGui::Separator();
        actor->editor();
    }
    else
    {
        ImGui::Text("No actor selected.");
    }

    ImGui::End();
}

void ViewportUI::renderGraphicsManager()
{
    ImGuiIO &io = ImGui::GetIO();

    ImVec2 topRightPos = ImVec2(io.DisplaySize.x - 150.0f, 30.0f);

    ImGui::SetNextWindowPos(topRightPos, ImGuiCond_Always);

    ImGui::Begin("##", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);

    _attachedScene->getGraphicsManager()->editor();

    ImGui::End();
}

void ViewportUI::renderAssetsManager()
{
    // ImGuiIO &io = ImGui::GetIO();

    // ImVec2 topRightPos = ImVec2(30.0f, io.DisplaySize.y - 400.f);

    // ImGui::SetNextWindowPos(topRightPos, ImGuiCond_Always);

    // _showInspector = true;
    // ImGui::Begin("Inspector", &_showInspector, ImGuiWindowFlags_AlwaysAutoResize);
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

} // namespace cmx
