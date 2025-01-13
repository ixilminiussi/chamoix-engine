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
    _inputManager = std::make_unique<InputManager>(cmxWindow, shortcutsPath);
    _inputManager->load();
}

void CmxEditor::load(CmxWindow &cmxWindow)
{
    initInputManager(cmxWindow);

    _viewportActor = std::make_shared<ViewportActor>();
    _viewportUI = std::make_unique<ViewportUI>(_viewportActor);
    _viewportUI->initImGUI();

    _inputManager->bindAxis("viewport movement", &ViewportActor::onMovementInput, _viewportActor.get());
    _inputManager->bindAxis("viewport rotation", &ViewportActor::onMouseMovement, _viewportActor.get());
    _inputManager->bindButton("viewport toggle", &ViewportActor::select, _viewportActor.get());
}

void CmxEditor::attachScene(Scene *scene)
{
    scene->setCamera(_viewportActor->getCamera());

    _viewportUI->attachScene(scene);

    _active = true;
}

void CmxEditor::update(float dt)
{
    _inputManager->pollEvents(dt);
    _viewportActor->update(dt);
    _viewportUI->update(dt);
}

void CmxEditor::render(const FrameInfo &frameInfo)
{
    _viewportUI->render(frameInfo);
}

} // namespace cmx
