#pragma once

#include "cmx_world.h"

// lib
#include <memory>
#include <vulkan/vulkan_core.h>

namespace cmx
{

class Component
{
  public:
    Component() = default;
    ~Component() = default;

    virtual void update(float dt) {};
    virtual void render(class FrameInfo &, VkPipelineLayout) {
        // TODO: Debug render code
    };

    // for viewport
    virtual void renderSettings() {};

    void despawn();

    // getters and setters :: begin
    void setParent(class Actor *actor)
    {
        parent = actor;
    }

    Actor *getParent()
    {
        return parent;
    }
    // getters and setters :: end

    // friend functions
    friend void World::addComponent(std::shared_ptr<Component>);
    friend bool operator<(std::weak_ptr<Component>, std::shared_ptr<Component>);

    const char *name{"Component"};

  protected:
    class Actor *parent;

    int32_t renderZ{-1}; // decides whether or not to add component to render queue:
                         // -1: no
                         // >0: yes, render lower values first
};

inline bool operator<(std::weak_ptr<Component> awk, std::shared_ptr<Component> b)
{
    if (auto a = awk.lock())
    {
        return a->renderZ < b->renderZ;
    }
    return true;
}

} // namespace cmx
