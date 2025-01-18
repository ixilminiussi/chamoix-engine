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
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include <glm/fwd.hpp>
#include <spdlog/spdlog.h>

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

void ViewportUI::render(const class FrameInfo &frameInfo)
{
    // imgui new frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();

    renderTopBar();

    if (_showProjectSettings)
        renderProjectSettings();

    if (_showViewportSettings)
        renderViewportSettings();

    if (_showSceneTree)
        renderSceneTree();

    if (_showInspector)
        renderInspector();

    renderPlayButton();
    renderGraphicsManager();

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), frameInfo.commandBuffer);
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
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
}

void ViewportUI::renderViewportSettings()
{
    _showViewportSettings = true;
    ImGui::Begin("Viewport Settings", &_showViewportSettings, ImGuiWindowFlags_AlwaysAutoResize);

    if (CmxEditor *editor = CmxEditor::getInstance())
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

    std::vector<std::weak_ptr<Actor>> actorList{};
    _attachedScene->getAllActorsByType<Actor>(actorList);

    auto it = actorList.begin();
    int i = 0;

    // render existing actors
    while (it != actorList.end())
    {
        ImGui::PushID(i++);
        if (std::shared_ptr<Actor> actor = it->lock())
        {
            if (ImGui::Button(actor->name.c_str(), ImVec2(170.0f, 0.0f)))
            {
                _inspectedActor = *it;
                renderInspector();
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_MS_DELETE))
            {
                _attachedScene->removeActor(actor.get());
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

    if (std::shared_ptr<Actor> actor = _inspectedActor.lock())
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
    //         CmxEditor *editor = CmxEditor::getInstance();
    //         editor->toggle(0.f, 0);
    //     }
    //
    //     ImGui::End();
}

void ViewportUI::initImGUI()
{
    // 1: create descriptor pool for IMGUI
    // the size of the pool is very oversize, but it's copied from imgui demo itself.
    CmxDevice &cmxDevice = *RenderSystem::getDevice();
    CmxWindow &cmxWindow = *RenderSystem::_cmxWindow;

    _imguiPool = CmxDescriptorPool::Builder(cmxDevice)
                     .setMaxSets(1000)
                     .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, 1000)
                     .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
                     .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000)
                     .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000)
                     .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000)
                     .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000)
                     .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
                     .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000)
                     .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000)
                     .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000)
                     .addPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000)
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
    init_info.RenderPass = RenderSystem::_cmxRenderer->getSwapChainRenderPass();

    ImGui_ImplVulkan_Init(&init_info);

    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("editor/JetBrainsMonoNL-Regular.ttf", 16.0f);
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

} // namespace cmx
