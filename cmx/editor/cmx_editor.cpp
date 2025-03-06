#include "cmx_editor.h"

// cmx
#include "cmx_actor.h"
#include "cmx_game.h"
#include "cmx_input_manager.h"
#include "cmx_viewport_actor.h"
#include "cmx_viewport_ui.h"

// std
#include <memory>

namespace cmx
{

Editor *Editor::_instance{nullptr};
bool Editor::_active{true};

Editor *Editor::getInstance()
{
    if (_instance == nullptr)
    {
        _instance = new Editor();
        return _instance;
    }

    return _instance;
}

Editor::Editor()
{
}

void Editor::initInputManager(Window &cmxWindow, const std::string &shortcutsPath)
{
    _inputManager = std::make_unique<InputManager>(cmxWindow, shortcutsPath);
    _inputManager->load();
}

void Editor::load(Window &cmxWindow)
{
    initInputManager(cmxWindow);

    _viewportActor = std::make_unique<ViewportActor>();
    _viewportUI = std::make_unique<ViewportUI>();
    _viewportUI->initImGUI();

    _inputManager->bindAxis("viewport movement", &ViewportActor::onMovementInput, _viewportActor.get());
    _inputManager->bindAxis("viewport rotation", &ViewportActor::onMouseMovement, _viewportActor.get());
    _inputManager->bindButton("viewport toggle", &ViewportActor::select, _viewportActor.get());
    _inputManager->bindButton("editor toggle", &Editor::toggle, this);
    _inputManager->bindButton("translate mode", &ViewportUI::guizmoToTranslate, _viewportUI.get());
    _inputManager->bindButton("scale mode", &ViewportUI::guizmoToScale, _viewportUI.get());
    _inputManager->bindButton("rotate mode", &ViewportUI::guizmoToRotate, _viewportUI.get());
    _inputManager->bindButton("duplicate selected", &ViewportUI::duplicateSelected, _viewportUI.get());
    _inputManager->bindAxis("movement speed update", &ViewportActor::updateMoveSpeed, _viewportActor.get());
}

void Editor::attachScene(Scene *scene)
{
    _scene = scene;

    _active = false;
    _scene->setCamera(_viewportActor->getCamera());
    _active = true;

    _viewportUI->attachScene(_scene);

    _active = true;
}

void Editor::toggle(float dt, int)
{
    spdlog::info("toggled");
    if (_active)
    {
        _viewportUI->saveState();
        _scene->saveAs("editor/temp.xml");
        _scene->unload();
        _scene->getGame()->getInputManager()->unbindAll();

        _active = false;
        _viewportActor->lock();

        _scene->loadFrom("editor/temp.xml");
    }
    else
    {
        _scene->unload();
        _scene->getGame()->getInputManager()->unbindAll();

        _active = true;
        _scene->loadFrom("editor/temp.xml");
        _active = false;
        _scene->setCamera(_viewportActor->getCamera());
        _viewportUI->reloadState();

        _active = true;
        _viewportActor->unlock();
    }
}

void Editor::update(float dt)
{
    _inputManager->pollEvents(dt);

    if (_active)
    {
        _viewportActor->update(dt);
    }
}

void Editor::render(const FrameInfo &frameInfo)
{
    _viewportUI->render(frameInfo);
}

} // namespace cmx
