#ifndef CMX_HUD_COMPONENT
#define CMX_HUD_COMPONENT

#include <cmx_component.h>
#include <cmx_drawable.h>
#include <cmx_frame_info.h>
#include <cmx_register.h>

namespace cmx
{

class HudComponent : public Component, public virtual Drawable
{
  public:
    HudComponent();
    ~HudComponent();

    CLONEABLE(HudComponent)

    void onAttach() override;

  protected:
    class Texture *_texture{nullptr};
};

REGISTER_COMPONENT(cmx::HudComponent)

} // namespace cmx

#endif
