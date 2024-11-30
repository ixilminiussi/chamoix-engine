#pragma once

#include "cmx_device.h"
#include "cmx_pipeline.h"
#include "cmx_world.h"

// std
#include <memory>
#include <vulkan/vulkan_core.h>

namespace cmx
{

class DefaultRenderSystem
{
  public:
    DefaultRenderSystem(CmxDevice &, VkRenderPass);
    ~DefaultRenderSystem();

    DefaultRenderSystem(const DefaultRenderSystem &) = delete;
    DefaultRenderSystem &operator=(const DefaultRenderSystem &) = delete;

    virtual void render(VkCommandBuffer, std::vector<std::weak_ptr<Component>> &, const class CameraComponent &);

  private:
    void createPipelineLayout();
    void createPipeline(VkRenderPass);

    CmxDevice &cmxDevice;
    std::unique_ptr<CmxPipeline> cmxPipeline;
    VkPipelineLayout pipelineLayout;
};
} // namespace cmx
