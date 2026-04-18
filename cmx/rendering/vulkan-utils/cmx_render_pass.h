#ifndef CMX_RENDER_PASS
#define CMX_RENDER_PASS

// lib
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

// std
#include <cstddef>
#include <optional>
#include <vector>
#include <vulkan/vulkan_structs.hpp>

namespace cmx
{

struct AttachmentInfo
{
    vk::ImageAspectFlags aspect{vk::ImageAspectFlagBits::eColor};
    vk::ClearValue clearValue{{0.f, 0.f, 0.f, 1.0}};
    vk::Format format{vk::Format::eR16G16B16A16Snorm};
    vk::ImageUsageFlags usage{};
    vk::ImageLayout initial{vk::ImageLayout::eUndefined}, final{vk::ImageLayout::eUndefined};
    vk::AttachmentLoadOp loadOp{vk::AttachmentLoadOp::eClear};
    vk::AttachmentStoreOp storeOp{vk::AttachmentStoreOp::eStore};
};

struct SubpassInfo
{
    std::vector<uint32_t> colorAttachmentIndices;
    std::optional<uint32_t> depthAttachmentIndex;
    std::vector<uint32_t> inputAttachmentIndices;
};

struct RenderTarget
{
    vk::ClearValue clearValue;
    vk::Format format;
    vk::Image image;
    vk::ImageView imageView;
    vk::Sampler sampler;
    vk::DeviceMemory deviceMemory;
    size_t descriptorSetID;

    void free(class Device *);
};

class RenderPass
{
  public:
    RenderPass(class Device *, const vk::Extent2D &resolution, const std::vector<AttachmentInfo> &,
               const std::vector<SubpassInfo> &);
    ~RenderPass();

    void free(class Device *);
    void buildRenderTargets(class Device *, const vk::Extent2D &resolution, const std::vector<AttachmentInfo> &,
                            const std::vector<SubpassInfo> &);

    RenderPass(const RenderPass &) = delete;
    RenderPass &operator=(const RenderPass &) = delete;

    void updateAspectRatio(class Device *, const vk::Extent2D &);
    void beginRender(vk::CommandBuffer) const;
    void endRender(vk::CommandBuffer) const;

    const std::vector<RenderTarget> &getRenderTargets()
    {
        return _renderTargets;
    }

    vk::RenderPass getRenderPass() const
    {
        return _renderPass;
    };
    vk::Framebuffer getFrameBuffer() const
    {
        return _framebuffer;
    }
    const vk::Extent2D &getResolution() const
    {
        return _resolution;
    }

  private:
    std::vector<RenderTarget> _renderTargets;

    const std::vector<AttachmentInfo> _attachmentInfos;
    const std::vector<SubpassInfo> _subpassInfos;

    vk::Extent2D _resolution;
    vk::RenderPass _renderPass;
    vk::Framebuffer _framebuffer;

    bool _freed{false};
};

} // namespace cmx

#endif
