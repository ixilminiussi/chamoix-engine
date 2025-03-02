#ifndef CMX_HUD_MATERIAL
#define CMX_HUD_MATERIAL

#include "cmx_material.h"

namespace cmx
{

class HudMaterialSystem : public MaterialSystem
{
  public:
    using MaterialSystem::MaterialSystem;

    void initialize() override;
    void free() override;

  private:
    void createPipelineLayout(std::vector<vk::DescriptorSetLayout>) override;
    void createPipeline(vk::RenderPass) override;

    void render(const struct FrameInfo *, std::vector<std::shared_ptr<class Component>> &,
                class GraphicsManager *) override;
    virtual void editor(int i) override;

    std::unique_ptr<Buffer> _dummyBuffer;
};

class HudMaterial : public Material
{
  public:
    virtual void onAttach() override;

    virtual void render() override;
    virtual void editor() override;
};

} // namespace cmx

#endif
