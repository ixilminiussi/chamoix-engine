#pragma once

// cmx
#include "cmx_buffer.h"
#include "cmx_camera_component.h"
#include "cmx_scene.h"
#include "cmx_viewport_ui_component.h"

// lib
#include <glm/ext/matrix_float4x4.hpp>
#include <vulkan/vulkan_core.h>

// std
#include <memory>
#include <set>

namespace cmx
{

struct SimplePushConstantData
{
    glm::mat4 modelMatrix{1.f};
    glm::mat4 normalMatrix{1.f};
};

class RenderSystem
{
  public:
    RenderSystem(class CmxWindow &);
    ~RenderSystem();

    RenderSystem(const RenderSystem &) = delete;
    RenderSystem &operator=(const RenderSystem &) = delete;

    virtual void initialize();
    virtual void drawScene(class std::weak_ptr<CameraComponent>, const std::vector<std::shared_ptr<class Component>> &);

    friend void ViewportUIComponent::initImGUI(RenderSystem *);

    std::unique_ptr<class CmxDevice> cmxDevice;

  private:
    void createPipelineLayout(VkDescriptorSetLayout);
    void createPipeline(VkRenderPass);

    virtual FrameInfo *beginRender(class CameraComponent *);
    virtual void render(class FrameInfo *, const std::vector<std::shared_ptr<class Component>> &);
    virtual void endRender();

    std::unique_ptr<class CmxRenderer> cmxRenderer;
    std::unique_ptr<class CmxPipeline> cmxPipeline;
    class CmxWindow *cmxWindow;

    std::unique_ptr<class CmxDescriptorPool> globalPool{};

    VkPipelineLayout pipelineLayout;
    VkCommandBuffer commandBuffer;

    std::vector<std::unique_ptr<CmxBuffer>> uboBuffers;
    std::vector<VkDescriptorSet> globalDescriptorSets;

    // warning flags
    bool noCameraFlag{false};
};
} // namespace cmx