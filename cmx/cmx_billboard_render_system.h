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
    void free() override;

  private:
    void createPipelineLayout(std::vector<vk::DescriptorSetLayout>) override;
    void createPipeline(vk::RenderPass) override;

    void render(const class FrameInfo *, std::vector<std::shared_ptr<class Component>> &,
                class GraphicsManager *) override;
    virtual void editor(int i) override;

    std::unique_ptr<Buffer> _dummyBuffer;
};

} // namespace cmx

#endif
