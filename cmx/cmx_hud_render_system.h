#ifndef CMX_HUD_RENDER_SYSTEM
#define CMX_HUD_RENDER_SYSTEM

// cmx
#include "cmx_buffer.h"
#include "cmx_render_system.h"

// lib
#include <glm/ext/matrix_float4x4.hpp>
#include <vulkan/vulkan_core.h>

// std
#include <memory>

namespace cmx
{

class HudRenderSystem : public RenderSystem
{
  public:
    using RenderSystem::RenderSystem;
    ~HudRenderSystem() = default;

    void initialize() override;
    void free() override;

  private:
    void createPipelineLayout(std::vector<vk::DescriptorSetLayout>) override;
    void createPipeline(vk::RenderPass) override;

    void render(const struct FrameInfo *, std::vector<std::shared_ptr<class Component>> &,
                class GraphicsManager *) override;
    virtual void editor(int i) override;

    std::unique_ptr<Buffer> _dummyBuffer;
};

} // namespace cmx

#endif
