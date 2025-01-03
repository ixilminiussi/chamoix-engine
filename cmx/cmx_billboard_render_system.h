#ifndef CMX_BILLBOARD_RENDER_SYSTEM
#define CMX_BILLBOARD_RENDER_SYSTEM

// cmx
#include "cmx_buffer.h"
#include "cmx_render_system.h"

// lib
#include <glm/ext/matrix_float4x4.hpp>
#include <vulkan/vulkan_core.h>

// std
#include <memory>
#include <set>

namespace cmx
{

struct BillboardPushConstant
{
    glm::vec4 position;
    glm::vec4 color;
    glm::vec2 scale;
};

class BillboardRenderSystem : public RenderSystem
{
  public:
    using RenderSystem::RenderSystem;
    ~BillboardRenderSystem() = default;

    void initialize() override;

  private:
    void createPipelineLayout(VkDescriptorSetLayout) override;
    void createPipeline(VkRenderPass) override;

    void render(class FrameInfo *, std::shared_ptr<class Component>) override;

    std::unique_ptr<CmxBuffer> _dummyBuffer;
};

} // namespace cmx

#endif
