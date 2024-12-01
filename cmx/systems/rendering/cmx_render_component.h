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
    RenderComponent();

    ~RenderComponent() = default;

    void render(class FrameInfo &, VkPipelineLayout) override;
    void setModel(std::shared_ptr<class CmxModel>);

  private:
    std::shared_ptr<class CmxModel> cmxModel;
};

} // namespace cmx
