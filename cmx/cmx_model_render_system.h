#ifndef CMX_MODEL_RENDER_SYSTEM
#define CMX_MODEL_RENDER_SYSTEM

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

struct SimplePushConstantData
{
    glm::mat4 modelMatrix{1.f};
    glm::mat4 normalMatrix{1.f};
};

class ModelRenderSystem : public RenderSystem
{
  public:
    using RenderSystem::RenderSystem;
    ~ModelRenderSystem() = default;

    void initialize() override;

  protected:
    void createPipelineLayout(VkDescriptorSetLayout) override;
    void createPipeline(VkRenderPass) override;

    void render(class FrameInfo *, std::vector<std::shared_ptr<class Component>> &, class GraphicsManager *) override;
};

} // namespace cmx

#endif
