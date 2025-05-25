#ifndef CMX_RENDER_SYSTEM
#define CMX_RENDER_SYSTEM

// cmx
#include "cmx_descriptors.h"
#include "cmx_light_environment.h"
#include "cmx_viewport_ui.h"

// lib
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

#define MAX_POINT_LIGHTS 10
#define MAX_SAMPLER_SETS 200

namespace cmx
{

struct DirectionalLightCompact
{
    glm::mat4 projection{1.f};
    glm::mat4 view{1.f};
    glm::vec4 color{};
    glm::vec4 direction{};
};

struct PointLightCompact
{
    glm::vec4 position{}; // ignore w
    glm::vec4 color{};    // a is intensity
};

struct GlobalUbo
{
    glm::mat4 projection{1.f};
    glm::mat4 view{1.f};
    glm::vec4 ambientLight{1.f};
    glm::vec4 cameraPos{0.f};
    DirectionalLightCompact sun{};
    PointLightCompact pointLights[MAX_POINT_LIGHTS];
    int numLights;
};

class RenderSystem
{
  public:
    static RenderSystem *getInstance();
    ~RenderSystem();

    RenderSystem(const RenderSystem &) = delete;
    RenderSystem &operator=(const RenderSystem &) = delete;

    void checkAspectRatio(class Camera *);
    struct FrameInfo *beginCommandBuffer();
    void beginRender(struct FrameInfo *, const class LightEnvironment *) const;
    void endRender(struct FrameInfo *) const;
    void beginPostProcess(struct FrameInfo *) const;
    void endPostProcess(struct FrameInfo *) const;
    void writeUbo(struct FrameInfo *, struct GlobalUbo *);

    void closeWindow();

    size_t createSamplerDescriptor(vk::ImageView, vk::Sampler);
    void freeSamplerDescriptor(size_t descriptorSetID);
    vk::DescriptorSet &getSamplerDescriptorSet(size_t index);

    friend void ViewportUI::initImGUI();

    class Device *getDevice();

    vk::DescriptorSetLayout getGlobalSetLayout()
    {
        return _globalSetLayout->getDescriptorSetLayout();
    };
    vk::DescriptorSetLayout getSamplerDescriptorSetLayout()
    {
        return _samplerDescriptorSetLayout->getDescriptorSetLayout();
    };
    class Renderer *getRenderer()
    {
        return _renderer.get();
    };
    const class GraphicsManager *getActiveGraphicsManager() const
    {
        return _activeGraphicsManager;
    }
    void setActiveGraphicsManager(const class GraphicsManager *graphicsManager)
    {
        _activeGraphicsManager = graphicsManager;
    }
    vk::RenderPass getRenderPass() const;

    vk::Extent2D getResolution() const;

    class RenderPass *getGBuffer()
    {
        return _gBuffer.get();
    }
    std::array<class RenderPass *, 2> getSSAOBuffer()
    {
        return _ssaoBuffers;
    }
    void drawSSAO(struct FrameInfo *frameInfo) const;
#ifndef NDEBUG
    class RenderPass *getViewport()
    {
        return _viewport.get();
    }
#endif

  private:
    RenderSystem();
    void initializeUbo();

    void createGBuffer();
    std::unique_ptr<class RenderPass> _gBuffer;

    void createSSAOBuffers();
    std::array<class Material *, 2> _ssaoMaterials;
    std::array<class RenderPass *, 2> _ssaoBuffers;
    const class GraphicsManager *_activeGraphicsManager;

#ifndef NDEBUG
    void createViewport();

    std::unique_ptr<class RenderPass> _viewport;
#endif

    std::unique_ptr<class DescriptorPool> _globalPool{};

    bool _visible{true};

    std::unique_ptr<class Renderer> _renderer;
    std::unique_ptr<class Device> _device;
    class Window *_window;
    vk::CommandBuffer _commandBuffer;
    std::vector<std::unique_ptr<class Buffer>> _uboBuffers;
    std::vector<vk::DescriptorSet> _globalDescriptorSets;

    std::unique_ptr<DescriptorSetLayout> _globalSetLayout;

    std::unique_ptr<class DescriptorPool> _samplerDescriptorPool;
    std::unique_ptr<class DescriptorSetLayout> _samplerDescriptorSetLayout;
    std::vector<vk::DescriptorSet> _samplerDescriptorSets;

    static RenderSystem *_instance;
};

} // namespace cmx

#endif
