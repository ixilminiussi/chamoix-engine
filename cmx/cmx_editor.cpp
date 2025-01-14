#include "cmx_editor.h"

// cmx
#include "cmx_input_manager.h"
#include "cmx_viewport_actor.h"
#include "cmx_viewport_ui.h"

// std
#include <memory>

namespace cmx
{

CmxEditor *CmxEditor::_instance{nullptr};
bool CmxEditor::_active{false};

CmxEditor *CmxEditor::getInstance()
{
    if (_instance == nullptr)
    {
        _instance = new CmxEditor();
        return _instance;
    }

    return _instance;
}

CmxEditor::CmxEditor()
{
}

void CmxEditor::initInputManager(CmxWindow &cmxWindow, const std::string &shortcutsPath)
{
    _inputManager = std::make_shared<InputManager>(cmxWindow, shortcutsPath);
    _inputManager->load();
}

void CmxEditor::load(CmxWindow &cmxWindow)
{
    initInputManager(cmxWindow);

    _viewportActor = std::make_shared<ViewportActor>();
    _viewportUI = std::make_unique<ViewportUI>(_viewportActor, _inputManager);
    _viewportUI->initImGUI();

    _inputManager->bindAxis("viewport movement", &ViewportActor::onMovementInput, _viewportActor.get());
    _inputManager->bindAxis("viewport rotation", &ViewportActor::onMouseMovement, _viewportActor.get());
    _inputManager->bindButton("viewport toggle", &ViewportActor::select, _viewportActor.get());
    _inputManager->bindButton("editor toggle", &CmxEditor::toggle, this);
}

void CmxEditor::attachScene(Scene *scene)
{
    _scene = scene;
    _scene->setCamera(_viewportActor->getCamera());

    _viewportUI->attachScene(_scene);

    _active = true;
}

void CmxEditor::toggle(float dt, int)
{
    spdlog::info("toggled");
    if (_active)
    {
        _scene->saveAs("editor/temp.xml");
        _active = false;
        _viewportActor->lock();
    }
    else
    {
        _scene->unload();
        _scene->loadFrom("editor/temp.xml");
        _active = true;
        _viewportActor->unlock();
        _scene->setCamera(_viewportActor->getCamera());
    }
}

void CmxEditor::update(float dt)
{
    _inputManager->pollEvents(dt);
    _viewportActor->update(dt);
}

void CmxEditor::render(const FrameInfo &frameInfo)
{
    _viewportUI->render(frameInfo);
}

} // namespace cmx
