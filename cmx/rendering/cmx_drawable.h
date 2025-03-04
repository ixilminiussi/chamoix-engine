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

    unsigned int getMaterialID() const;
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
    void load(tinyxml2::XMLElement *);

    void render(struct FrameInfo &, DrawOption *drawOption) const;

    void setDrawOption(const DrawOption &, size_t index = 0);
    void setMaterial(const std::string &, size_t index = 0);
    void setTextures(const std::vector<std::string>, size_t index = 0);
    void setModel(const std::string &, size_t index = 0);

    const std::vector<DrawOption const *> getDrawOptions() const;

  private:
    std::map<size_t, DrawOption> _drawOptions;
    bool _isVisible{true};

    class Actor **_parentP{nullptr};
};

} // namespace cmx

#endif
