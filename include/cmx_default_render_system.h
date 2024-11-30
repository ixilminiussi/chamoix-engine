#pragma once

#include "cmx_device.h"
#include "cmx_pipeline.h"
#include "cmx_world.h"

// std
#include <memory>
#include <vulkan/vulkan_core.h>

namespace cmx
{

class CmxDefaultRenderSystem
{
  public:
    CmxDefaultRenderSystem(CmxDevice &, VkRenderPass);
    ~CmxDefaultRenderSystem();

    CmxDefaultRenderSystem(const CmxDefaultRenderSystem &) = delete;
    CmxDefaultRenderSystem &operator=(const CmxDefaultRenderSystem &) = delete;

    virtual void render(VkCommandBuffer, std::vector<std::weak_ptr<Component>> &, const class CameraComponent &);

  private:
    void createPipelineLayout();
    void createPipeline(VkRenderPass);

    CmxDevice &cmxDevice;
    std::unique_ptr<CmxPipeline> cmxPipeline;
    VkPipelineLayout pipelineLayout;
};
} // namespace cmx
