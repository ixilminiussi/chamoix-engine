#include "cmx_scene.h"

// cmx
#include "cmx_actor.h"
#include "cmx_assets_manager.h"
#include "cmx_camera.h"
#include "cmx_component.h"
#include "cmx_drawable.h"
#include "cmx_editor.h"
#include "cmx_game.h"
#include "cmx_graphics_manager.h"
#include "cmx_light_environment.h"
#include "cmx_physics_body.h"
#include "cmx_physics_manager.h"
#include "cmx_register.h"
#include "cmx_utils.h"

// std
#include <memory>
#include <stdexcept>

// lib
#include <spdlog/spdlog.h>
#include <tinyxml2.h>

namespace cmx
{

Scene::Scene(const std::string &xmlPath, class Game *game, const std::string &name)
    : _xmlPath{std::string(GAME_FILES) + xmlPath}, _game{game}, name{name} {};

Scene::~Scene()
{
    for (auto &pair : _actors)
    {
        delete pair.second;
    }
}

void Scene::load(bool skipAssets)
{
    loadFrom(_xmlPath, skipAssets);
}

void Scene::loadFrom(const std::string &filepath, bool skipAssets, bool absolute)
{
    if (!skipAssets)
    {
        _assetsManager = std::make_unique<AssetsManager>(this);
    }

    _xmlPath = absolute ? filepath : std::string(GAME_FILES) + filepath;

    _graphicsManager = std::make_unique<GraphicsManager>();
    _physicsManager = std::make_unique<PhysicsManager>();
    _lightEnvironment = std::make_unique<LightEnvironment>();

    _activeCamera = std::make_shared<Camera>();
    _activeCamera->setViewDirection(glm::vec3{0.f}, glm::vec3{0.f, 0.f, 1.f});

    Register &cmxRegister = Register::getInstance();

    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(_xmlPath.c_str()) == tinyxml2::XML_SUCCESS)
    {
        spdlog::info("Scene: Loading new scene from `{0}`...", _xmlPath);

        tinyxml2::XMLElement *rootElement = doc.RootElement();
        name = std::string(rootElement->Attribute("name"));

        if (!skipAssets)
        {
            _assetsManager->load(rootElement);
        }
        _lightEnvironment->load(rootElement);
        _graphicsManager->load(rootElement, _assetsManager.get());

        tinyxml2::XMLElement *actorElement = rootElement->FirstChildElement("actor");
        while (actorElement)
        {
            try
            {
                cmxRegister.spawnActor(actorElement->Attribute("type"), this, actorElement->Attribute("name"))
                    ->load(actorElement);
            }
            catch (std::out_of_range e)
            {
                throw std::out_of_range(std::string("Scene ") + name + std::string(": No actor type <") +
                                        actorElement->Attribute("type") + std::string("> in register of actors"));
            }

            actorElement = actorElement->NextSiblingElement("actor");
        }

        spdlog::info("Scene {0}: Succesfully loaded new scene!", name);
    }
    else
    {
        spdlog::warn("Scene {0}: Couldn't load scene from `{1}`, {2}", name, _xmlPath, doc.ErrorStr());
    }
}

void Scene::unload(bool keepAssets)
{
    spdlog::info("Scene {0}: Unloading scene...", name);
    if (!keepAssets)
    {
        _assetsManager->unload();
        delete _assetsManager.release();
    }
    delete _graphicsManager.release();
    delete _physicsManager.release();
    delete _lightEnvironment.release();

    for (auto &pair : _actors)
    {
        delete pair.second;
    }
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
    catch (const std::out_of_range &)
    {
        spdlog::warn("Scene {0}: Attempt to get actor from invalid id: {1}'", name, id);
    }

    return actor;
}

void Scene::setCamera(std::shared_ptr<Camera> camera, bool bForce)
{
#ifndef NDEBUG
    if (Editor::isActive() && !bForce)
    {
        return;
    }
#endif
    _activeCamera = camera;
    spdlog::info("Scene: new active Camera");
}

void Scene::addActor(class Actor *actor)
{
#ifndef NDEBUG
    // expensive operation so we only use it in debug mode
    if (getActorByName(actor->name) != nullptr)
    {
        spdlog::warn("Scene {0}: An actor with name <{1}> already exists", name, actor->name);
        while (getActorByName(actor->name) != nullptr)
        {
            actor->name = incrementNumberInParentheses(actor->name);
        }
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
    _graphicsManager->drawRenderQueue(getCamera(), _lightEnvironment.get());
}

void Scene::removeActor(Actor *actor)
{
    auto it = _actors.find(actor->_id);
    if (it == _actors.end())
    {
        spdlog::warn("Scene {0}: Attempt to remove non-managed actor <{1}>'", name, actor->name);
        return;
    }

    for (auto [name, component] : actor->getComponents())
    {
        removeComponent(component);
    }

    _actors.erase(it);
    delete actor;

    spdlog::info("Scene {0}: Removed actor <{1}>", name, actor->name);
}

bool Scene::renameActor(Actor *actor, std::string name)
{
    if (name.compare("") == 0)
        return false;

    for (auto &[uint32_t, actor] : _actors)
    {
        if (name.compare(actor->name) == 0)
        {
            return false;
        }
    }

    actor->name = name;
    return true;
}

void Scene::updateActors(float dt)
{
    auto it = _actors.begin();

    while (it != _actors.end())
    {
        if ((*it).second == nullptr)
        {
            goto erase;
        }
        if ((*it).second->markedForDeletion())
        {
            spdlog::info("Scene {0}: Removed actor <{1}>", name, (*it).second->name);
        erase:
            it = _actors.erase(it);
            continue;
        }

        (*it).second->update(dt);
        it++;
    }
}

void Scene::removeComponent(std::shared_ptr<Component> component)
{
    if (std::shared_ptr<Drawable> drawable = std::dynamic_pointer_cast<Drawable>(component))
    {
        _graphicsManager->remove(drawable.get());
    }
    if (std::shared_ptr<PhysicsBody> physicsBody = std::dynamic_pointer_cast<PhysicsBody>(component))
    {
        _physicsManager->remove(physicsBody.get());
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
            if (std::shared_ptr<Drawable> drawable = std::dynamic_pointer_cast<Drawable>(component))
            {
                _graphicsManager->remove(drawable.get());
            }
            if (std::shared_ptr<PhysicsBody> physicsBody = std::dynamic_pointer_cast<PhysicsBody>(component))
            {
                _physicsManager->remove(physicsBody.get());
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

tinyxml2::XMLElement &Scene::saveAs(const char *filepath, bool absolute)
{
    spdlog::info("Scene {0}: saving scene to `{1}`...", name, filepath);

    _xmlPath = absolute ? std::string(filepath) : std::string(GAME_FILES) + std::string(filepath);

    tinyxml2::XMLDocument doc;
    doc.InsertFirstChild(doc.NewDeclaration());

    tinyxml2::XMLElement *sceneElement = doc.NewElement("scene");
    sceneElement->SetAttribute("name", name.c_str());

    _assetsManager->save(doc, sceneElement);
    _lightEnvironment->save(doc, sceneElement);
    _graphicsManager->save(doc, sceneElement);

    for (auto &actorPair : _actors)
    {
        actorPair.second->save(doc, sceneElement);
    }

    doc.InsertEndChild(sceneElement);

    if (doc.SaveFile((std::string(GAME_FILES) + _xmlPath).c_str()) != tinyxml2::XML_SUCCESS)
    {
        spdlog::error("Scene {0}: {1}", name, doc.ErrorStr());
    };

    spdlog::info("Scene {0}: Succesfully saved to `{1}`!", name, _xmlPath);
    return *sceneElement;
}

} // namespace cmx
