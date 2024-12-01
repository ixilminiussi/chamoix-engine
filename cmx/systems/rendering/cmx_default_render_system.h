#pragma once

#include "cmx_device.h"
#include "cmx_pipeline.h"
#include "cmx_world.h"

// std
#include <glm/ext/matrix_float4x4.hpp>
#include <memory>
#include <vulkan/vulkan_core.h>

namespace cmx
{

struct SimplePushConstantData
{
    glm::mat4 modelMatrix{1.f};
    glm::mat4 normalMatrix{1.f};
};

class DefaultRenderSystem
{
  public:
    DefaultRenderSystem(CmxDevice &, VkRenderPass, VkDescriptorSetLayout globalSetLayout);
    ~DefaultRenderSystem();

    DefaultRenderSystem(const DefaultRenderSystem &) = delete;
    DefaultRenderSystem &operator=(const DefaultRenderSystem &) = delete;

    virtual void render(class FrameInfo &, std::vector<std::weak_ptr<Component>> &renderQueue);

  private:
    void createPipelineLayout(VkDescriptorSetLayout);
    void createPipeline(VkRenderPass);

    CmxDevice &cmxDevice;
    std::unique_ptr<CmxPipeline> cmxPipeline;
    VkPipelineLayout pipelineLayout;
};
} // namespace cmx
