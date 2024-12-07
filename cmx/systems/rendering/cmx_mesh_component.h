#pragma once

#include "cmx_component.h"
#include "cmx_model.h"
#include "tinyxml2.h"

// lib
#include <vulkan/vulkan_core.h>

// std
#include <memory>

namespace cmx
{

class MeshComponent : public Component
{
  public:
    MeshComponent();
    MeshComponent(std::shared_ptr<class CmxModel>);

    ~MeshComponent() = default;

    void render(class FrameInfo &, VkPipelineLayout) override;
    void setModel(std::shared_ptr<class CmxModel>);

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &, tinyxml2::XMLElement *) override;

  private:
    std::shared_ptr<class CmxModel> cmxModel;
};

} // namespace cmx
