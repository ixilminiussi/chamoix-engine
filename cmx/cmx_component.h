#ifndef CMX_COMPONENT
#define CMX_COMPONENT

// cmx
#include "cmx_scene.h"
#include "cmx_transform.h"

// lib
#include <tinyxml2.h>
#include <vulkan/vulkan_core.h>

// std
#include <memory>

namespace cmx
{

class Component : public std::enable_shared_from_this<Component>
{
  public:
    Component() = default;
    ~Component();

    virtual void onDetach() {};
    virtual void onAttach() {};
    virtual void update(float dt) {};
    virtual void render(class FrameInfo &, VkPipelineLayout);

    virtual tinyxml2::XMLElement &save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentComponent);
    virtual void load(tinyxml2::XMLElement *);

    // for viewport
    virtual void editor(int i);

    void despawn();

    std::string getType();

    // getters and setters :: begin
    void setParent(std::weak_ptr<class Actor> actor);

    Actor *getParent()
    {
        if (auto parent = _parent.lock())
        {
            return parent.get();
        }
        return nullptr;
    }

    int8_t getRenderZ()
    {
        return _renderZ;
    }

    uint8_t getRequestedRenderSystem()
    {
        return _requestedRenderSystem;
    }

    const Transform &getRelativeTransform();
    Transform getAbsoluteTransform();

    class Scene *getScene()
    {
        return _scene;
    }

    bool getVisible();
    // getters and setters :: end

    // friend functions
    friend bool operator<(std::shared_ptr<Component>, std::shared_ptr<Component>);

    std::string name;

  protected:
    std::weak_ptr<class Actor> _parent;
    class Scene *_scene;
    Transform _transform{};

    bool _isVisible{true};
    int8_t _renderZ{-1}; // decides whether or not to add component to render queue:
                         // -1: no
                         // >0: yes, render lower values first
    uint8_t _requestedRenderSystem{0u};
};

inline bool operator<(std::shared_ptr<Component> a, std::shared_ptr<Component> b)
{
    return a->_renderZ < b->_renderZ;
}

} // namespace cmx

#endif
