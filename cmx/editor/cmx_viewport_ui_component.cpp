#include "cmx_viewport_ui_component.h"

// cmx
#include "cmx_actor.h"
#include "cmx_descriptors.h"
#include "cmx_frame_info.h"
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

    if (showViewportSettings)
        renderViewportSettings();

    if (showWorldTree)
        renderWorldTree();

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), frameInfo.commandBuffer);
}

void ViewportUIComponent::renderTopBar()
{
    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu("Settings"))
    {
        if (ImGui::MenuItem("Viewport Settings", "Ctrl+V"))
        {
            renderViewportSettings();
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Toolbar"))
    {
        if (ImGui::MenuItem("World Tree", "Ctrl+T"))
        {
            renderWorldTree();
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

void ViewportUIComponent::renderWorldTree()
{
    showWorldTree = true;
    ImGui::Begin("World Tree", &showWorldTree);

    std::vector<std::weak_ptr<Actor>> actorList{};
    getParent()->getWorld()->getAllActorsByType<Actor>(actorList);

    for (auto actorWk : actorList)
    {
        if (std::shared_ptr<Actor> actor = actorWk.lock())
        {
            if (actor->name == getParent()->name)
                continue;
            if (ImGui::CollapsingHeader(actor->name.c_str()))
            {
                actor->renderSettings();
            }
        }
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

} // namespace cmx
