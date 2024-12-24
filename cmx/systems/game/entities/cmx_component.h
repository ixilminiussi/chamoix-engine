#pragma once

// cmx
#include "cmx_scene.h"

// lib
#include "tinyxml2.h"
#include <vulkan/vulkan_core.h>

// std
#include <memory>

namespace cmx
{

class Component
{
  public:
    Component() = default;
    ~Component() = default;

    virtual void update(float dt) {};
    virtual void render(class FrameInfo &, VkPipelineLayout);

    virtual tinyxml2::XMLElement &save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentComponent);

    // for viewport
    virtual void renderSettings(int i) {};

    void despawn();

    std::string getType();

    // getters and setters :: begin
    void setParent(class Actor *actor)
    {
        _parent = actor;
    }

    Actor *getParent()
    {
        return _parent;
    }
    // getters and setters :: end

    // friend functions
    friend void Scene::addComponent(std::shared_ptr<Component>);
    friend bool operator<(std::weak_ptr<Component>, std::shared_ptr<Component>);

    std::string name;

  protected:
    class Actor *_parent = nullptr;

    int32_t _renderZ{-1}; // decides whether or not to add component to render queue:
                          // -1: no
                          // >0: yes, render lower values first
};

inline bool operator<(std::weak_ptr<Component> awk, std::shared_ptr<Component> b)
{
    if (auto a = awk.lock())
    {
        return a->_renderZ < b->_renderZ;
    }
    return true;
}

} // namespace cmx
