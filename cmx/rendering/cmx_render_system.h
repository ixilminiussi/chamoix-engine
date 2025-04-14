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
    void beginRender(class FrameInfo *, const class LightEnvironment *) const;
    void endRender(class FrameInfo *) const;
    void beginPostProcess(class FrameInfo *) const;
    void endPostProcess(class FrameInfo *) const;
    void writeUbo(class FrameInfo *, class GlobalUbo *);

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
    vk::RenderPass getRenderPass()
    {
        return _renderPass;
    };
    size_t *getSamplerDescriptorSetIDs()
    {
        return _samplerDescriptorSetIDs;
    }

  private:
    RenderSystem();
    void initializeUbo();

    void initializeScreenTextures();
    void createImages();
    void createImageViews();
    void createRenderPass();
    void createFrameBuffer();
    void createSamplers();

    void transitionImages(class FrameInfo *) const;

    void freeImages();

    vk::Format _colorFormat;
    vk::Image _colorImage;
    vk::ImageView _colorImageView;
    vk::Sampler _colorSampler;
    size_t _samplerDescriptorSetIDs[3];
    vk::DeviceMemory _colorImageMemory;
    vk::Image _normalImage;
    vk::ImageView _normalImageView;
    vk::Sampler _normalSampler;
    vk::DeviceMemory _normalImageMemory;
    vk::Image _depthImage;
    vk::ImageView _depthImageView;
    vk::Sampler _depthSampler;
    vk::DeviceMemory _depthImageMemory;
    vk::Extent2D _imageResolution;
    vk::RenderPass _renderPass;
    vk::Framebuffer _framebuffer;

    std::unique_ptr<class DescriptorPool> _globalPool{};

    bool _visible{true};
    bool _freed{false};

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
