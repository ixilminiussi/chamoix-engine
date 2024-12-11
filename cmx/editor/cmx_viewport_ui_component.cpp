#include "cmx_viewport_ui_component.h"

// cmx
#include "cmx_actor.h"
#include "cmx_component.h"
#include "cmx_descriptors.h"
#include "cmx_frame_info.h"
#include "cmx_game.h"
#include "cmx_renderer.h"

// lib
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include <glm/fwd.hpp>
#include <memory>
#include <spdlog/spdlog.h>

// std
#include <limits>

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

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), frameInfo.commandBuffer);
}

void ViewportUIComponent::renderTopBar()
{
    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("Save", "Ctrl+S"))
        {
            parent->getScene()->save();
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Settings"))
    {
        if (ImGui::MenuItem("Viewport Settings", "Ctrl+V"))
        {
            renderViewportSettings();
        }
        if (ImGui::MenuItem("Project Settings", "Ctrl+V"))
        {
            renderProjectSettings();
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Toolbar"))
    {
        if (ImGui::MenuItem("Scene Tree", "Ctrl+T"))
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

    ImGui::End();
}

void ViewportUIComponent::renderProjectSettings()
{
    static int activeTab = 0;

    Game *game = getParent()->getScene()->getGame();

    showProjectSettings = true;
    ImGui::SetNextWindowSize({500, 1000});
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
    ImGui::SetNextWindowSize({300, 500});
    ImGui::Begin("Scene Tree", &showSceneTree);

    std::vector<std::weak_ptr<Actor>> actorList{};
    getParent()->getScene()->getAllActorsByType<Actor>(actorList);

    int i = 0;
    for (auto actorWk : actorList)
    {
        if (std::shared_ptr<Actor> actor = actorWk.lock())
        {
            if (actor->name == getParent()->name)
                continue;
            if (ImGui::CollapsingHeader(actor->name.c_str()))
            {
                actor->renderSettings(i);
            }
        }
        i++;
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
}

tinyxml2::XMLElement &ViewportUIComponent::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentComponent)
{
    tinyxml2::XMLElement &componentElement = Component::save(doc, parentComponent);

    return componentElement;
}

} // namespace cmx
