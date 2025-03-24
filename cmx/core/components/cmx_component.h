#ifndef CMX_COMPONENT
#define CMX_COMPONENT

#define CLONEABLE_COMPONENT(Type)                                                                                      \
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

class Component : public virtual Transformable
{
  public:
    Component() = default;
    ~Component();

    virtual void onDetach() {};
    virtual void onAttach() {};

    [[nodiscard]] virtual std::shared_ptr<Component> clone() const
    {
        return std::make_shared<Component>(*this);
    }

    virtual void update(float dt) {};
    virtual void render(const struct FrameInfo &, vk::PipelineLayout);

    virtual tinyxml2::XMLElement &save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const;
    virtual void load(tinyxml2::XMLElement *);

    // for viewport
    virtual void editor(int i);

    void despawn();

    [[nodiscard]] std::string getType() const;

    // getters and setters :: begin
    void setParent(class Actor *actor);

    class Actor *getParent()
    {
        return _parent;
    }

    [[nodiscard]] const Transform &getLocalSpaceTransform() const override;
    [[nodiscard]] Transform getWorldSpaceTransform(int depth = -1) const override;

    class Scene *getScene()
    {
        return _scene;
    }
    // getters and setters :: end

    std::string name;

  protected:
    class Actor *_parent{nullptr};
    class Scene *_scene{nullptr};
};

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
