#include "cmx_scene.h"

// cmx
#include "cmx/cmx_camera.h"
#include "cmx/cmx_editor.h"
#include "cmx_actor.h"
#include "cmx_assets_manager.h"
#include "cmx_component.h"
#include "cmx_game.h"
#include "cmx_graphics_manager.h"
#include "cmx_physics_component.h"
#include "cmx_physics_manager.h"
#include "cmx_register.h"

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
    loadFrom(_xmlPath);
}

void Scene::loadFrom(const std::string &filepath)
{
    _assetsManager = std::make_unique<AssetsManager>(this);
    _graphicsManager = std::make_unique<GraphicsManager>(getGame()->getRenderSystems());
    _physicsManager = std::make_unique<PhysicsManager>();

    _activeCamera = std::make_shared<Camera>();
    _activeCamera->setViewDirection(glm::vec3{0.f}, glm::vec3{0.f, 0.f, 1.f});

    Register *cmxRegister = Register::getInstance();

    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(filepath.c_str()) == tinyxml2::XML_SUCCESS)
    {
        spdlog::info("Scene: Loading new scene from `{0}`...", filepath);

        tinyxml2::XMLElement *rootElement = doc.RootElement();
        name = std::string(rootElement->Attribute("name"));

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
                spdlog::error("Scene {0}: No actor type <{1}> in register of actors", name,
                              actorElement->Attribute("type"));
            }
            actorElement = actorElement->NextSiblingElement("actor");
        }

        spdlog::info("Scene {0}: Succesfully loaded new scene!", name);
    }
    else
    {
        spdlog::error("Scene {0}: Couldn't load scene from `{1}`, {2}", name, filepath, doc.ErrorStr());
    }
}

void Scene::unload()
{
    spdlog::info("Scene {0}: Unloading scene...", name);
    _assetsManager->unload();
    delete _assetsManager.release();
    delete _graphicsManager.release();
    delete _physicsManager.release();

    _actors = std::unordered_map<uint32_t, Actor *>{};
    _components = std::vector<std::shared_ptr<Component>>{};
    spdlog::info("Scene {0}: Succesfully unloaded scene!");
}

Actor *Scene::getActorByName(const std::string &name)
{
    for (auto &pair : _actors)
    {
        if (pair.second->name == name)
        {
            return pair.second;
        }
    }

    return nullptr;
}

Actor *Scene::getActorByID(uint32_t id)
{
    Actor *actor = nullptr;
    try
    {
        actor = _actors.at(id);
    }
    catch (const std::out_of_range &e)
    {
        spdlog::warn("Scene {0}: Attempt to get actor from invalid id: {1}'", name, id);
    }

    return actor;
}

void Scene::setCamera(std::shared_ptr<Camera> camera)
{
#ifndef NDEBUG
    if (CmxEditor::isActive())
    {
        return;
    }
#endif
    _activeCamera = camera;
    spdlog::info("Scene: new active Camera");
}

void Scene::addActor(class Actor *actor)
{
#ifdef DEBUG
#else
    // expensive operation so we only use it in debug mode
    if (getActorByName(actor->name) != nullptr)
    {
        spdlog::warn("Scene {0}: An actor with name <{1}> already exists", name, actor->name);
        return;
    }
#endif

    if (!actor)
    {
        spdlog::warn("Scene {0}: Attempt at adding uninitialized actor", name);
        return;
    }

    _actors[actor->_id] = actor;
    actor->onBegin();
    spdlog::info("Scene {0}: Added new Actor <{1}>", name, actor->name);
}

void Scene::update(float dt)
{
    _physicsManager->executeStep(dt);
    updateActors(dt);
    updateComponents(dt);
}

void Scene::render()
{
    _graphicsManager->drawComponents(getCamera());
}

void Scene::removeActor(Actor *actor)
{
    try
    {
        delete _actors.at(actor->_id);
    }
    catch (const std::out_of_range &e)
    {
        spdlog::warn("Scene {0}: Attempt to remove non-managed actor <{1}>'", name, actor->name);
    }

    _actors.erase(actor->_id);

    spdlog::info("Scene {0}: Removed actor <{1}>", name, actor->name);
}

void Scene::updateActors(float dt)
{
    auto it = _actors.begin();

    while (it != _actors.end())
    {
        if ((*it).second == nullptr)
        {
            it = _actors.erase(it);
            continue;
        }

        (*it).second->update(dt);
        it++;
    }
}

void Scene::removeComponent(std::shared_ptr<Component> component)
{
    if (component->getRenderZ() >= 0)
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
    spdlog::info("Scene {0}: Added new component <{1}->{2}>", name, component->getParent()->name, component->name);

    if (component->getRenderZ() >= 0)
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
            if (component->getRenderZ() >= 0)
            {
                _graphicsManager->removeFromQueue(*it);
            }
            if (std::shared_ptr<PhysicsComponent> physicsComponent =
                    std::dynamic_pointer_cast<PhysicsComponent>(component))
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
    spdlog::info("Scene {0}: saving scene to `{1}`...", name, filepath);

    tinyxml2::XMLDocument doc;
    doc.InsertFirstChild(doc.NewDeclaration());

    tinyxml2::XMLElement *sceneElement = doc.NewElement("scene");
    sceneElement->SetAttribute("name", name.c_str());

    _assetsManager->save(doc, sceneElement);

    for (auto &actorPair : _actors)
    {
        actorPair.second->save(doc, sceneElement);
    }

    doc.InsertEndChild(sceneElement);

    if (doc.SaveFile(filepath) != tinyxml2::XML_SUCCESS)
    {
        spdlog::error("Scene {0}: {1}", name, doc.ErrorStr());
    };

    spdlog::info("Scene {0}: Succesfully saved to `{1}`!", name, filepath);
    return *sceneElement;
}

} // namespace cmx
