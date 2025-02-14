#ifndef CMX_HUD_COMPONENT
#define CMX_HUD_COMPONENT

#include "cmx/cmx_frame_info.h"
#include <cmx/cmx_component.h>

namespace cmx
{

class HudComponent : public Component
{
  public:
    HudComponent();
    ~HudComponent();

    CLONEABLE(HudComponent)

    void onAttach() override;
    void render(const FrameInfo &, vk::PipelineLayout) override;

    void setTexture(const std::string &name);
    std::string getTextureName() const;

  protected:
    class Texture *_texture{nullptr};
};

} // namespace cmx

#endif
