#ifndef CMX_BILLBOARD_COMPONENT
#define CMX_BILLBOARD_COMPONENT

// cmx
#include "cmx_component.h"
#include "cmx_drawable.h"

// lib
#include <vulkan/vulkan_core.h>

namespace cmx
{

class BillboardComponent : public Component, public virtual Drawable
{
  public:
    BillboardComponent() : Drawable{&_parent} {};
    ~BillboardComponent() = default;

    CLONEABLE(BillboardComponent)

    void onAttach() override;

    void editor(int i) override;
    void load(tinyxml2::XMLElement *componentElement) override;
    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const override;
};

} // namespace cmx

#endif
