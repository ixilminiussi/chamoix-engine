#ifndef CMX_RENDER_SYSTEM
#define CMX_RENDER_SYSTEM

#include "cmx/cmx_descriptors.h"
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>
#define SHADED_RENDER_SYSTEM 0u
#define BILLBOARD_RENDER_SYSTEM 1u
#define EDGE_RENDER_SYSTEM 2u
#define NULL_RENDER_SYSTEM 255u

#define TRANSPARENT_BILLBOARD_Z 100u
#define DEFAULT_Z 1u
#define DEBUG_BILLBOARD_Z 10u

#define MAX_POINT_LIGHTS 10

// cmx
#include "cmx_viewport_ui.h"

namespace cmx
{

struct PointLight
{
    glm::vec4 position; // ignore w
    glm::vec4 color;    // a is intensity
};

struct GlobalUbo
{
    glm::mat4 projection{1.f};
    glm::mat4 view{1.f};
    glm::vec4 ambientLight{1.f, 1.f, 1.f, .02f};
    PointLight pointLights[MAX_POINT_LIGHTS];
    int numLights;
};

class RenderSystem
{
  public:
    RenderSystem();
    virtual ~RenderSystem();

    RenderSystem(const RenderSystem &) = delete;
    RenderSystem &operator=(const RenderSystem &) = delete;

    static void checkAspectRatio(class Camera *);
    static FrameInfo *beginRender(class Camera *, PointLight pointLights[MAX_POINT_LIGHTS], int numLights);
    static void endRender();

    virtual void free();
    static void closeWindow();

    static unsigned int createSamplerDescriptor(vk::ImageView, vk::Sampler);
    static vk::DescriptorSet &getSamplerDescriptorSet(unsigned int index);

    virtual void initialize() = 0;
    virtual void render(const class FrameInfo *, std::vector<std::shared_ptr<class Component>> &,
                        class GraphicsManager *) = 0;

    friend void ViewportUI::initImGUI();

    virtual void editor(int i);

    static class Device *getDevice();
    vk::PipelineLayout getPipelineLayout() const
    {
        return _pipelineLayout;
    }

  protected:
    static void initializeUbo();

    virtual void createPipelineLayout(std::vector<vk::DescriptorSetLayout>) = 0;
    virtual void createPipeline(vk::RenderPass) = 0;

    std::unique_ptr<class Pipeline> _pipeline;
    std::unique_ptr<class DescriptorPool> _globalPool{};
    vk::PipelineLayout _pipelineLayout;

    bool _visible{true};
    bool _freed{false};

    static std::unique_ptr<class Renderer> _renderer;
    static std::unique_ptr<class Device> _device;
    static class Window *_window;
    static vk::CommandBuffer _commandBuffer;
    static std::vector<std::unique_ptr<class Buffer>> _uboBuffers;
    static std::vector<vk::DescriptorSet> _globalDescriptorSets;

    static std::unique_ptr<class DescriptorPool> _samplerDescriptorPool;
    static std::unique_ptr<class DescriptorSetLayout> _samplerDescriptorSetLayout;
    static std::vector<vk::DescriptorSet> _samplerDescriptorSets;

    static bool _uboInitialized;

    static uint8_t _activeSystem;
};

} // namespace cmx

#endif
