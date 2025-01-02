#include "cmx_scene.h"

// cmx
#include "cmx/cmx_physics_component.h"
#include "cmx_actor.h"
#include "cmx_assets_manager.h"
#include "cmx_component.h"
#include "cmx_game.h"
#include "cmx_graphics_manager.h"
#include "cmx_physics_manager.h"
#include "cmx_register.h"
#include "cmx_viewport_actor.h"

// std
#include <memory>
#include <stdexcept>

// lib
#include <spdlog/spdlog.h>
#include <tinyxml2.h>

namespace cmx
{

Scene::Scene(const std::string &xmlPath, class Game *game, const std::string &name)
    : _xmlPath{xmlPath}, _game{game}, name{name} {};

Scene::~Scene()
{
}

void Scene::load()
{
    _assetsManager = std::make_shared<AssetsManager>(this);
    _graphicsManager = std::make_shared<GraphicsManager>(getGame()->getRenderSystem());
    _physicsManager = std::make_shared<PhysicsManager>();

    Register *cmxRegister = Register::getInstance();

    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(_xmlPath.c_str()) == tinyxml2::XML_SUCCESS)
    {
        spdlog::info("Scene: Loading new scene from {0}...", _xmlPath);

        tinyxml2::XMLElement *rootElement = doc.RootElement();

        _assetsManager->load(rootElement);

        tinyxml2::XMLElement *actorElement = rootElement->FirstChildElement("actor");
        while (actorElement)
        {
            try
            {
                cmxRegister
                    ->spawnActor(std::string(actorElement->Attribute("type")), this,
                                 std::string(actorElement->Attribute("name")))
                    ->load(actorElement);
            }
            catch (std::out_of_range e)
            {
                spdlog::error("Scene: No actor type '{0}' in register of actors", actorElement->Attribute("type"));
            }
            actorElement = actorElement->NextSiblingElement("actor");
        }

        spdlog::info("Scene: Completed loading new scene...");
    }
    else
    {
        spdlog::error("Scene: Couldn't load scene from {0}, {1}", _xmlPath, doc.ErrorStr());
    }
}

void Scene::unload()
{
}

std::weak_ptr<Actor> Scene::getActorByName(const std::string &name)
{
    for (auto pair : _actors)
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
        actor = _actors.at(id);
    }
    catch (const std::out_of_range &e)
    {
        spdlog::warn("Scene '{0}': Attempt to get actor from invalid id: '{1}''", name, id);
    }

    return actor;
}

void Scene::setCamera(std::shared_ptr<Camera> camera)
{
    _activeCamera = camera;
    spdlog::info("Scene: new active Camera");
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
    _actors[actor->_id] = actor;
    spdlog::info("Scene '{0}': Added new Actor '{1}'", name, actor->name);

    return actor;
}

void Scene::update(float dt)
{
    _physicsManager->executeStep(dt);
    updateActors(dt);
    updateComponents(dt);
    _graphicsManager->drawComponents(getCamera());
}

void Scene::removeActor(Actor *actor)
{
    try
    {
        _actors.at(actor->_id) = nullptr;
    }
    catch (const std::out_of_range &e)
    {
        spdlog::warn("Scene '{0}': Attempt to remove non-managed actor '{1}''", name, actor->name);
    }

    _actors.erase(actor->_id);

    spdlog::info("Scene '{0}': Removed actor '{1}'", name, actor->name);
}

void Scene::updateActors(float dt)
{
    for (auto pair : _actors)
    {
        pair.second->update(dt);
    }
}

void Scene::removeComponent(std::shared_ptr<Component> component)
{
#ifndef NDEBUG
    if (component->getRenderZ() >= -1)
#else
    if (component->getRenderZ() >= 0)
#endif
    {
        _graphicsManager->removeFromQueue(component);
    }

    auto it = _components.begin();

    while (it != _components.end())
    {
        if (*it == component)
        {
            _components.erase(it);
            return;
        }
        it++;
    }
}

void Scene::addComponent(std::shared_ptr<Component> component)
{
    _components.push_back(component);
    spdlog::info("Scene '{0}': Added new component '{1}->{2}'", name, component->getParent()->name, component->name);

#ifndef NDEBUG
    if (component->getRenderZ() >= -1)
#else
    if (component->getRenderZ() >= 0)
#endif
    {
        _graphicsManager->addToQueue(component);
    }
}

void Scene::updateComponents(float dt)
{
    auto it = _components.begin();

    while (it != _components.end())
    {
        std::shared_ptr<Component> component = *it;
        // if component is deleted, remove it from our list
        if (component->getParent() == nullptr)
        {
#ifndef NDEBUG
            if (component->getRenderZ() >= -1)
#else
            if (component->getRenderZ() >= 0)
#endif
            {
                _graphicsManager->removeFromQueue(*it);
            }
            if (auto physicsComponent = std::dynamic_pointer_cast<PhysicsComponent>(component))
            {
                _physicsManager->remove(physicsComponent);
            }
            it = _components.erase(it);
            continue;
        }

        component->update(dt);

        it++;
    }
}

tinyxml2::XMLElement &Scene::save()
{
    return saveAs(_xmlPath.c_str());
}

tinyxml2::XMLElement &Scene::saveAs(const char *filepath)
{
    spdlog::info("Scene: saving scene to {0}...", filepath);

    tinyxml2::XMLDocument doc;
    doc.InsertFirstChild(doc.NewDeclaration());

    tinyxml2::XMLElement *sceneElement = doc.NewElement("scene");
    sceneElement->SetAttribute("name", name.c_str());

    _assetsManager->save(doc, sceneElement);

    for (auto actorPair : _actors)
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
