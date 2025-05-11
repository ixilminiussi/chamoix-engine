#ifndef NDEBUG
#include "cmx_editor.h"

// cmx
#include "cmx_game.h"
#include "cmx_input_manager.h"
#include "cmx_viewport_actor.h"
#include "cmx_viewport_ui.h"

// std
#include "ImGuizmo.h"
#include <immintrin.h>
#include <memory>
#include <spdlog/fmt/bundled/base.h>

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
    _inputManager->bindButton("editor exit", &Editor::leave, this);
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

void Editor::declarePlayIntent()
{
    _playIntent = true;
}

void Editor::play()
{
    if (_active)
    {
        _playIntent = false;
        _viewportUI->saveState();
        _scene->saveAs(".editor/temp.xml", false);
        _scene->unload(true);
        _scene->getGame()->getInputManager()->unbindAll();

        _active = false;
        _viewportActor->lock();

        _scene->load(true);
    }
}

void Editor::leave(float dt, int)
{
    if (!_active)
    {
        _scene->unload(true);
        _scene->getGame()->getInputManager()->unbindAll();

        _active = true;
        _scene->load(true);
        _scene->setCamera(_viewportActor->getCamera(), true);
        _viewportUI->reloadState();

        _viewportActor->unlock();
    }
}

glm::vec2 Editor::getSceneViewportSize()
{
    glm::vec2 size;
    if (_viewportUI != nullptr)
    {
        ImVec2 imguiSize = _viewportUI->getSceneViewportSize();
        size.x = std::max(100.f, imguiSize.x);
        size.y = std::max(100.f, imguiSize.y);
    }
    else
    {
        size = {100.f, 100.f};
    }

    return size;
}

bool Editor::isViewportActorSelected()
{
    if (_viewportUI == nullptr || _viewportActor == nullptr || !_active)
    {
        return true;
    }

    return _viewportActor->isSelected();
}

bool Editor::isViewportActorHovered()
{
    if (_viewportUI == nullptr || !_active)
    {
        return true;
    }

    return (_viewportUI->isHoveringSceneViewport() && !ImGuizmo::IsOver());
}

void Editor::update(float dt)
{
    _inputManager->pollEvents(dt);

    if (_active)
    {
        _viewportActor->update(dt);
        _viewportUI->update();

        if (_playIntent)
            play();
    }
}

void Editor::render(const FrameInfo &frameInfo)
{
    _viewportUI->render(frameInfo);
}

} // namespace cmx

#endif
