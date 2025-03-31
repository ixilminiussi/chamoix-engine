#ifndef CMX_DRAWABLE
#define CMX_DRAWABLE

// cmx
#include "cmx_assets_manager.h"
#include "cmx_transform.h"

// lib
#include <vulkan/vulkan.hpp>

// std
#include <map>

namespace cmx
{

struct DrawOption
{
    class Material *material{nullptr};
    class Model *model{nullptr};
    std::vector<class Texture *> textures{};

    size_t getMaterialID() const;
};

class Drawable : public virtual Transformable
{
  public:
    Drawable(class Actor **parentP);
    virtual ~Drawable() = default;

    Actor *getParentActor() const
    {
        return *_parentP;
    };

    void editor(int i);
    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const;
    void load(tinyxml2::XMLElement *parentElement);

    void render(struct FrameInfo &, DrawOption *drawOption) const;

    void setDrawOption(const DrawOption &, size_t index = 0);
    void setMaterial(const char *, size_t index = 0);
    void setTextures(const std::vector<const char *>, size_t index = 0);
    void setModel(const char *, size_t index = 0);
    bool isVisible() const
    {
        return _isVisible;
    }

    const std::vector<DrawOption const *> getDrawOptions() const;

  private:
    std::map<size_t, DrawOption> _drawOptions;

    class Actor **_parentP{nullptr};

  protected:
    bool _isVisible{true};
};

} // namespace cmx

#endif
