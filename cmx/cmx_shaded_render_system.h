#ifndef CMX_SHADED_RENDER_SYSTEM
#define CMX_SHADED_RENDER_SYSTEM

// cmx
#include "cmx_render_system.h"

// lib
#include <glm/ext/matrix_float4x4.hpp>
#include <vulkan/vulkan_core.h>

// std
#include <memory>
#include <vector>

namespace cmx
{

struct SimplePushConstantData
{
    glm::mat4 modelMatrix{1.f};
    glm::mat4 normalMatrix{1.f};
};

class ShadedRenderSystem : public RenderSystem
{
  public:
    using RenderSystem::RenderSystem;
    ~ShadedRenderSystem() = default;

    void initialize() override;

  protected:
    void createPipelineLayout(VkDescriptorSetLayout) override;
    void createPipeline(VkRenderPass) override;

    void render(const class FrameInfo *, std::vector<std::shared_ptr<class Component>> &,
                class GraphicsManager *) override;
    virtual void editor(int i) override;
};

} // namespace cmx

#endif
