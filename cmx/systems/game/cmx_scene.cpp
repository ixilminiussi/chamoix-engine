#include "cmx_scene.h"

// cmx
#include "cmx_actor.h"
#include "cmx_camera_component.h"
#include "cmx_game.h"

// std
#include <memory>
#include <stdexcept>

// lib
#include "misc/cmx_registers.h"
#include "tinyxml2.h"
#include <spdlog/spdlog.h>

namespace cmx
{

void Scene::load()
{
    graphicsManager = std::unique_ptr<GraphicsManager>(new GraphicsManager(getGame()->getRenderSystem()));

    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(xmlPath.c_str()) == tinyxml2::XML_SUCCESS)
    {
        spdlog::info("Scene: Loading new scene from {0}...", xmlPath);

        tinyxml2::XMLElement *rootElement = doc.RootElement();

        assetsManager->load(rootElement);

        tinyxml2::XMLElement *actorElement = rootElement->FirstChildElement("actor");
        while (actorElement)
        {
            std::shared_ptr<Actor> actor =
                cmx::reg::loadActor(actorElement->Attribute("type"), this, actorElement->Attribute("name"));
            actor->load(actorElement);

            actorElement = actorElement->NextSiblingElement("actor");
        }

        spdlog::info("Scene: Completed loading new scene...");
    }
    else
    {
        spdlog::error("Scene: Couldn't load scene from {0}, {1}", xmlPath, doc.ErrorStr());
    }
}

void Scene::unload()
{
}

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

std::shared_ptr<Actor> Scene::addActor(std::shared_ptr<Actor> actor)
{
#ifdef DEBUG
#else
    // expensive operation so we only use it in debug mode
    if (!getActorByName(actor->name).expired())
    {
        spdlog::warn("Scene '{0}': An actor with name '{1}' already exists", name, actor->name);

        return (getActorByName(actor->name).lock());
    }
#endif

    actor->onBegin();
    actors[actor->id] = actor;
    spdlog::info("Scene '{0}': Added new Actor '{1}'", name, actor->name);

    return actor;
}

void Scene::update(float dt)
{
    updateActors(dt);
    updateComponents(dt);
    graphicsManager->drawComponents(getCamera());
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
        graphicsManager->addToQueue(component);
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

tinyxml2::XMLElement &Scene::save()
{
    return saveAs(xmlPath.c_str());
}

tinyxml2::XMLElement &Scene::saveAs(const char *filepath)
{
    spdlog::info("Scene: saving scene to {0}...", filepath);

    tinyxml2::XMLDocument doc;
    doc.InsertFirstChild(doc.NewDeclaration());

    tinyxml2::XMLElement *sceneElement = doc.NewElement("scene");
    sceneElement->SetAttribute("name", name.c_str());

    assetsManager->save(doc, sceneElement);

    for (auto actorPair : actors)
    {
        if (std::shared_ptr<ViewportActor> viewport = std::dynamic_pointer_cast<ViewportActor>(actorPair.second))
        {
            continue;
        }
        actorPair.second->save(doc, sceneElement);
    }

    doc.InsertEndChild(sceneElement);

    if (doc.SaveFile(filepath) != tinyxml2::XML_SUCCESS)
    {
        spdlog::error("FILE SAVING: {0}", doc.ErrorStr());
    };

    spdlog::info("Scene: saving success!", filepath);
    return *sceneElement;
}

} // namespace cmx
