#pragma once

#include "cmx_component.h"
#include "cmx_model.h"

// lib
#include <vulkan/vulkan_core.h>

// std
#include <memory>

namespace cmx
{

class RenderComponent : public Component
{
  public:
    RenderComponent(std::shared_ptr<class CmxModel>);

    RenderComponent() = delete;
    ~RenderComponent() = default;

    void render(VkCommandBuffer, VkPipelineLayout) override;

  private:
    std::shared_ptr<class CmxModel> cmxModel;
};

} // namespace cmx
