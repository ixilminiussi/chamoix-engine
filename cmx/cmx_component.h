#ifndef CMX_COMPONENT
#define CMX_COMPONENT

#define CLONEABLE(Type)                                                                                                \
    std::shared_ptr<Component> clone() const override                                                                  \
    {                                                                                                                  \
        return std::make_shared<Type>(*this);                                                                          \
    }

// cmx
#include "cmx_scene.h"
#include "cmx_transform.h"

// lib
#include <tinyxml2.h>
#include <vulkan/vulkan.hpp>

// std
#include <memory>

namespace cmx
{

class Component : public std::enable_shared_from_this<Component>, public Transformable
{
  public:
    Component() = default;
    ~Component();

    virtual void onDetach() {};
    virtual void onAttach() {};

    virtual std::shared_ptr<Component> clone() const
    {
        return std::make_shared<Component>(*this);
    }

    virtual void update(float dt) {};
    virtual void render(const struct FrameInfo &, vk::PipelineLayout);

    virtual tinyxml2::XMLElement &save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentComponent) const;
    virtual void load(tinyxml2::XMLElement *);

    // for viewport
    virtual void editor(int i);

    void despawn();

    std::string getType() const;

    // getters and setters :: begin
    void setParent(class Actor *actor);

    class Actor *getParent()
    {
        return _parent;
    }

    int8_t getRenderZ()
    {
        return _renderZ;
    }

    uint8_t getRequestedRenderSystem()
    {
        return _requestedRenderSystem;
    }

    const Transform &getLocalSpaceTransform() const override;
    Transform getWorldSpaceTransform() const override;

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
    class Actor *_parent{nullptr};
    class Scene *_scene{nullptr};

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

#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)
#define UNIQUE_ID CONCAT(__COUNTER__, __LINE__)

#define REGISTER_COMPONENT_INTERNAL(Type, ID)                                                                          \
    struct CONCAT(Registrar_, ID)                                                                                      \
    {                                                                                                                  \
        CONCAT(Registrar_, ID)()                                                                                       \
        {                                                                                                              \
            cmx::Register::getInstance().addComponent(#Type, []() { return std::make_shared<Type>(); });               \
        }                                                                                                              \
    };                                                                                                                 \
    [[maybe_unused]] inline CONCAT(Registrar_, ID) CONCAT(registrar_, ID){};

#define REGISTER_COMPONENT(Type) REGISTER_COMPONENT_INTERNAL(Type, UNIQUE_ID)

#endif
