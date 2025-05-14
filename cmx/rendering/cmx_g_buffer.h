#ifndef CMX_G_BUFFER
#define CMX_G_BUFFER

// lib
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

// std
#include <cstddef>
#include <vector>

namespace cmx
{

class GBuffer
{
  public:
    GBuffer();
    ~GBuffer();

    void free(class Device *);

    GBuffer(const GBuffer &) = delete;
    GBuffer &operator=(const GBuffer &) = delete;

    void updateAspectRatio(class Device *device, const vk::Extent2D &);
    void beginRender(struct FrameInfo *, const class LightEnvironment *) const;
    void endRender(struct FrameInfo *) const;

    vk::RenderPass getRenderPass()
    {
        return _renderPass;
    };
    size_t *getSamplerDescriptorSetIDs()
    {
        return _samplerDescriptorSetIDs;
    };

    void createTextures(vk::Extent2D resolution, class Device *);

  private:
    void createImages(class Device *);
    void createImageViews(class Device *);
    void createRenderPass(class Device *);
    void createFrameBuffer(class Device *);
    void createSamplers(class Device *);

    size_t _samplerDescriptorSetIDs[4];
    vk::Format _albedoFormat;
    vk::Image _albedoImage;
    vk::ImageView _albedoImageView;
    vk::Sampler _albedoSampler;
    vk::DeviceMemory _albedoImageMemory;
    vk::Image _normalImage;
    vk::ImageView _normalImageView;
    vk::Sampler _normalSampler;
    vk::DeviceMemory _normalImageMemory;
    vk::Image _shadowImage;
    vk::ImageView _shadowImageView;
    vk::Sampler _shadowSampler;
    vk::DeviceMemory _shadowImageMemory;
    vk::Image _depthImage;
    vk::ImageView _depthImageView;
    vk::Sampler _depthSampler;
    vk::DeviceMemory _depthImageMemory;
    vk::Extent2D _resolution;
    vk::RenderPass _renderPass;
    vk::Framebuffer _framebuffer;

    std::vector<vk::DescriptorSet> _samplerDescriptorSets;

    bool _freed{false};
};

} // namespace cmx

#endif
