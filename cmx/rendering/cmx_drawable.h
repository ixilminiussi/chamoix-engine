#ifndef CMX_DRAWABLE
#define CMX_DRAWABLE

// cmx
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
    uint8_t getIndex() const
    {
        return index;
    }

  private:
    uint8_t index{0};
};

class Drawable : public virtual Transformable
{
  public:
    Drawable(class Actor **parentP);
    ~Drawable();

    Actor *getParentActor() const
    {
        return *_parentP;
    };

    void editor();
    void load();
    void save();

    void render(const struct FrameInfo &, DrawOption *drawOption) const;

    void setDrawOption(const DrawOption &, uint8_t index = 0);
    void setMaterial(Material *, uint8_t index = 0);
    void setTextures(const std::vector<class Texture *>, uint8_t index = 0);
    void setModel(Model *, uint8_t index = 0);

    const std::map<uint8_t, DrawOption> &getDrawOptions() const;

  private:
    std::map<uint8_t, DrawOption> _drawOptions;
    bool _isVisible{true};

    class Actor **_parentP{nullptr};
};

} // namespace cmx

#endif
