#ifndef CMX_RENDER_SYSTEM
#define CMX_RENDER_SYSTEM

#define MODEL_RENDER_SYSTEM 1u
#define BILLBOARD_RENDER_SYSTEM 0u
#define NULL_RENDER_SYSTEM 255u

// cmx
#include "cmx_viewport_ui_component.h"

namespace cmx
{

struct GlobalUbo
{
    glm::mat4 projection{1.f};
    glm::mat4 view{1.f};
    glm::vec3 lightPosition{-1.f};
    glm::vec4 lightColor{1.f};
    glm::vec4 ambientLight{1.f, 1.f, 1.f, .02f};
};

class RenderSystem
{
  public:
    RenderSystem();
    virtual ~RenderSystem();

    RenderSystem(const RenderSystem &) = delete;
    RenderSystem &operator=(const RenderSystem &) = delete;

    static void checkAspectRatio(class Camera *);
    static FrameInfo *beginRender(class Camera *);
    static void endRender();

    virtual void initialize() = 0;
    virtual void render(class FrameInfo *, std::shared_ptr<class Component>) = 0;

    friend void ViewportUIComponent::initImGUI();

    static std::shared_ptr<class CmxDevice> getDevice();

  protected:
    virtual void createPipelineLayout(VkDescriptorSetLayout) = 0;
    virtual void createPipeline(VkRenderPass) = 0;

    std::unique_ptr<class CmxPipeline> _cmxPipeline;
    std::unique_ptr<class CmxDescriptorPool> _globalPool{};
    VkPipelineLayout _pipelineLayout;

    static VkCommandBuffer _commandBuffer;
    static std::unique_ptr<class CmxRenderer> _cmxRenderer;
    static CmxWindow *_cmxWindow;
    static std::shared_ptr<class CmxDevice> _cmxDevice;
    static std::vector<std::unique_ptr<class CmxBuffer>> _uboBuffers;
    static std::vector<VkDescriptorSet> _globalDescriptorSets;

    static uint8_t _activeSystem;
};

} // namespace cmx

#endif
