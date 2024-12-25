#ifndef CMX_RENDER_SYSTEM
#define CMX_RENDER_SYSTEM

// cmx
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

    std::unique_ptr<class CmxDevice> _cmxDevice;

  private:
    void createPipelineLayout(VkDescriptorSetLayout);
    void createPipeline(VkRenderPass);

    virtual FrameInfo *beginRender(class CameraComponent *);
    virtual void render(class FrameInfo *, const std::vector<std::shared_ptr<class Component>> &);
    virtual void endRender();

    std::unique_ptr<class CmxRenderer> _cmxRenderer;
    std::unique_ptr<class CmxPipeline> _cmxPipeline;
    class CmxWindow *_cmxWindow;

    std::unique_ptr<class CmxDescriptorPool> _globalPool{};

    VkPipelineLayout _pipelineLayout;
    VkCommandBuffer _commandBuffer;

    std::vector<std::unique_ptr<class CmxBuffer>> _uboBuffers;
    std::vector<VkDescriptorSet> _globalDescriptorSets;

    // warning flags
    bool _noCameraFlag{false};
};
} // namespace cmx

#endif
