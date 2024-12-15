#include "cmx_viewport_ui_component.h"

// cmx
#include "cmx_actor.h"
#include "cmx_component.h"
#include "cmx_descriptors.h"
#include "cmx_frame_info.h"
#include "cmx_game.h"
#include "cmx_registers.h"
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
#include <limits>
#include <memory>

namespace cmx
{

ViewportUIComponent::ViewportUIComponent(float &vpMoveSpeed, float &vpSensitivity)
    : viewportMovementSpeed{vpMoveSpeed}, viewportSensitivity{vpSensitivity}
{
    renderZ = std::numeric_limits<int32_t>::max(); // ensures it gets rendered at the very top
}

ViewportUIComponent::~ViewportUIComponent()
{
    ImGui_ImplVulkan_Shutdown();
}

void ViewportUIComponent::update(float dt)
{
    if (inputManager != nullptr)
    {
        inputManager->pollEvents(dt);
    }
}

void ViewportUIComponent::render(class FrameInfo &frameInfo, VkPipelineLayout pipelineLayout)
{
    // imgui new frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();

    renderTopBar();

    if (showProjectSettings)
        renderProjectSettings();

    if (showViewportSettings)
        renderViewportSettings();

    if (showSceneTree)
        renderSceneTree();

    if (showInspector)
        renderInspector();

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), frameInfo.commandBuffer);
}

void ViewportUIComponent::renderTopBar()
{
    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem(ICON_MS_SAVE " Save", "Ctrl+S"))
        {
            parent->getScene()->save();
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

void ViewportUIComponent::renderViewportSettings()
{
    showViewportSettings = true;
    ImGui::Begin("Viewport Settings", &showViewportSettings);

    ImGui::SliderFloat("movement speed", &viewportMovementSpeed, 0.0f, 100.0f);
    ImGui::SliderFloat("mouse sensitivity", &viewportSensitivity, 0.0f, 10.0f);

    ImGui::SeparatorText("Shortcuts");
    if (inputManager != nullptr)
    {
        inputManager->renderSettings();
    }

    ImGui::End();
}

void ViewportUIComponent::renderProjectSettings()
{
    static int activeTab = 0;

    Game *game = getParent()->getScene()->getGame();

    showProjectSettings = true;
    ImGui::Begin("Project Settings", &showProjectSettings);

    if (ImGui::BeginTabBar(""))
    {
        if (ImGui::BeginTabItem("Input Manager"))
        {
            activeTab = 0;

            game->getInputManager()->renderSettings();

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();

        ImGui::End();
    }
}

void ViewportUIComponent::renderSceneTree()
{
    showSceneTree = true;
    ImGui::Begin("Scene Tree", &showSceneTree);

    std::vector<std::weak_ptr<Actor>> actorList{};
    getParent()->getScene()->getAllActorsByType<Actor>(actorList);

    auto it = actorList.begin();
    int i = 0;

    // render existing actors
    while (it != actorList.end())
    {
        ImGui::PushID(i++);
        if (std::shared_ptr<Actor> actor = it->lock())
        {
            if (actor->name == getParent()->name)
            {
                it++;
                ImGui::PopID();
                continue;
            }
            if (ImGui::Button(actor->name.c_str()))
            {
                inspectedActor = *it;
                renderInspector();
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_MS_DELETE))
            {
                getParent()->getScene()->removeActor(actor.get());
                ImGui::PopID();
                continue;
            }
        }

        ImGui::PopID();
        it++;
    }

    // create new actor
    static const char *selected = reg::list[0];

    ImGui::PushID(i++);
    ImGui::SetNextItemWidth(172);
    if (ImGui::BeginCombo("##", selected))
    {
        for (const char *option : reg::list)
        {
            bool isSelected = (strcmp(selected, option) == 0);

            if (ImGui::Selectable(option, isSelected))
            {
                selected = option;
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
    ImGui::SetNextItemWidth(140);
    ImGui::InputText("##", buffer, 100);
    ImGui::SameLine();
    if (ImGui::Button(ICON_MS_ADD))
    {
        reg::loadActor(std::string(selected), getParent()->getScene(), buffer);
    }

    ImGui::End();
}

void ViewportUIComponent::renderInspector()
{
    showInspector = true;
    ImGui::Begin("Inspector", &showInspector);

    if (std::shared_ptr<Actor> actor = inspectedActor.lock())
    {
        actor->renderSettings();
    }
    else
    {
        ImGui::Text("No actor selected.");
    }

    ImGui::End();
}

void ViewportUIComponent::initImGUI(CmxDevice &cmxDevice, CmxWindow &cmxWindow, CmxRenderer &cmxRenderer)
{
    // 1: create descriptor pool for IMGUI
    // the size of the pool is very oversize, but it's copied from imgui demo itself.

    imguiPool = CmxDescriptorPool::Builder(cmxDevice)
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
    init_info.DescriptorPool = imguiPool->getDescriptorPool();
    init_info.MinImageCount = 3;
    init_info.ImageCount = 3;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.RenderPass = cmxRenderer.getSwapChainRenderPass();

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

void ViewportUIComponent::initInputManager(class CmxWindow &window, const std::string &shortcutsPath)
{
    inputManager = std::make_unique<InputManager>(window, shortcutsPath);
    inputManager->load();

    inputManager->bindAxis("viewport movement", &ViewportActor::onMovementInput, (ViewportActor *)(getParent()));
    inputManager->bindAxis("viewport rotation", &ViewportActor::onMouseMovement, (ViewportActor *)getParent());
    inputManager->bindButton("viewport toggle", &ViewportActor::select, (ViewportActor *)getParent());
}

tinyxml2::XMLElement &ViewportUIComponent::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentComponent)
{
    tinyxml2::XMLElement &componentElement = Component::save(doc, parentComponent);

    return componentElement;
}

} // namespace cmx
