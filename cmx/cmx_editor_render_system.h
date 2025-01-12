#ifndef CMX_EDITOR_RENDER_SYSTEM
#define CMX_EDITOR_RENDER_SYSTEM

// cmx
#include "cmx_render_system.h"

// lib
#include <glm/ext/matrix_float4x4.hpp>
#include <vulkan/vulkan_core.h>

// std
#include <memory>
#include <set>
#include <vector>

namespace cmx
{

class EditorRenderSystem : public RenderSystem
{
  public:
    using RenderSystem::RenderSystem;
    ~EditorRenderSystem() = default;

    void initialize() override;

  protected:
    void createPipelineLayout(VkDescriptorSetLayout) override;
    void createPipeline(VkRenderPass) override;

    void render(const class FrameInfo *, std::vector<std::shared_ptr<class Component>> &,
                class GraphicsManager *) override;
};

} // namespace cmx

#endif
