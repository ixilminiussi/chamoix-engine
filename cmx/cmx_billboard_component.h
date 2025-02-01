#ifndef CMX_BILLBOARD_COMPONENT
#define CMX_BILLBOARD_COMPONENT

#include "cmx_component.h"

// lib
#include <vulkan/vulkan_core.h>

namespace cmx
{

class BillboardComponent : public Component
{
  public:
    BillboardComponent();
    ~BillboardComponent() = default;

    void onAttach() override;

    void render(const class FrameInfo &, vk::PipelineLayout) override;

    void setTexture(const std::string &name);
    std::string getTextureName() const;

    void editor(int i) override;
    void load(tinyxml2::XMLElement *componentElement) override;
    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentComponent) override;

    const glm::vec3 &getHue()
    {
        return _hue;
    }

    void setHue(glm::vec3 hue)
    {
        _hue = hue;
    }

  private:
    glm::vec3 _hue{1.0f};

    class Texture *_texture{nullptr};
};

} // namespace cmx

#endif
