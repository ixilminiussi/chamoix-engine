#include "cmx_scene.h"

// cmx
#include "cmx_actor.h"
#include "cmx_camera_component.h"

// std
#include <memory>
#include <stdexcept>

// lib
#include "tinyxml2.h"
#include <spdlog/spdlog.h>

namespace cmx
{

std::weak_ptr<Actor> Scene::getActorByName(const std::string &name)
{
    for (auto pair : actors)
    {
        if (pair.second->name == name)
        {
            return pair.second;
        }
    }

    return std::weak_ptr<Actor>();
}

std::weak_ptr<Actor> Scene::getActorByID(uint32_t id)
{
    std::weak_ptr<Actor> actor;
    try
    {
        actor = actors.at(id);
    }
    catch (const std::out_of_range &e)
    {
        spdlog::warn("Scene '{0}': Attempt to get actor from invalid id: '{1}''", name, id);
    }

    return actor;
}

void Scene::setCamera(std::shared_ptr<class CameraComponent> camera)
{
    activeCamera = camera;
    spdlog::info("Scene: new active Camera is {0}->{1}", camera->getParent()->name, camera->name);
}

void Scene::addActor(std::shared_ptr<Actor> actor)
{
#ifdef DEBUG
#else
    // expensive operation so we only use it in debug mode
    if (!getActorByName(actor->name).expired())
    {
        spdlog::warn("Scene '{0}': An actor with name '{0}' already exists", name, actor->name);
    }
#endif

    actor->onBegin();
    actors[actor->id] = actor;
    spdlog::info("Scene '{0}': Added new Actor '{1}'", name, actor->name);
}

void Scene::removeActor(Actor *actor)
{
    try
    {
        actors.at(actor->id) = nullptr;
    }
    catch (const std::out_of_range &e)
    {
        spdlog::warn("Scene '{0}': Attempt to remove non-managed actor '{1}''", name, actor->name);
    }

    actors.erase(actor->id);

    spdlog::info("Scene '{0}': Removed actor '{1}'", name, actor->name);
}

void Scene::updateActors(float dt)
{
    for (auto pair : actors)
    {
        pair.second->update(dt);
    }
}

void Scene::addComponent(std::shared_ptr<Component> component)
{
    components.push_back(component);
    spdlog::info("Scene '{0}': Added new component '{1}->{2}'", name, component->getParent()->name, component->name);

#ifndef NDEBUG
    if (component->renderZ >= -1)
#else
    if (component->renderZ >= 0)
#endif
    {
        auto it = std::lower_bound(renderQueue.begin(), renderQueue.end(), component);
        renderQueue.insert(it, component);
        spdlog::info("Scene '{0}': Component '{1}' joins renderQueue with renderZ {2}", name, component->name,
                     component->renderZ);
    }
}

void Scene::updateComponents(float dt)
{
    for (auto i = components.begin(); i < components.end();)
    {
        // if component is deleted, remove it from our list
        if (i->expired())
        {
            i = components.erase(i);
            continue;
        }

        std::shared_ptr<Component> component = i->lock();
        if (component)
            component->update(dt);

        i++;
    }
}

tinyxml2::XMLElement &Scene::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parent)
{
    tinyxml2::XMLElement *sceneElement = doc.NewElement("scene");
    sceneElement->SetAttribute("name", name.c_str());
    parent->InsertEndChild(sceneElement);

    for (auto actorPair : actors)
    {
        actorPair.second->save(doc, sceneElement);
    }

    return *sceneElement;
}

} // namespace cmx
