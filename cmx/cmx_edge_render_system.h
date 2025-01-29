#ifndef CMX_EDGE_RENDER_SYSTEM
#define CMX_EDGE_RENDER_SYSTEM

// cmx
#include "cmx_render_system.h"

// lib
#include <glm/ext/matrix_float4x4.hpp>
#include <vulkan/vulkan.hpp>

// std
#include <memory>
#include <vector>

namespace cmx
{

struct EdgePushConstantData
{
    glm::mat4 modelMatrix{1.f};
    glm::vec3 color;
};

class EdgeRenderSystem : public RenderSystem
{
  public:
    using RenderSystem::RenderSystem;
    ~EdgeRenderSystem() = default;

    void initialize() override;

  protected:
    void createPipelineLayout(std::vector<vk::DescriptorSetLayout>) override;
    void createPipeline(vk::RenderPass) override;

    void render(const class FrameInfo *, std::vector<std::shared_ptr<class Component>> &,
                class GraphicsManager *) override;
    virtual void editor(int i) override;
};

} // namespace cmx

#endif
