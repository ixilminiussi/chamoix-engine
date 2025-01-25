#include "cmx_camera_component.h"

// cmx
#include "cmx/cmx_edge_render_system.h"
#include "cmx_actor.h"
#include "cmx_assets_manager.h"
#include "cmx_camera.h"
#include "cmx_editor.h"
#include "cmx_frame_info.h"
#include "cmx_model.h"
#include "cmx_render_system.h"

// lib
#include "imgui.h"

namespace cmx
{

CameraComponent::CameraComponent() : _camera{std::make_shared<Camera>()}
{
    _renderZ = DEBUG_BILLBOARD_Z;
    _requestedRenderSystem = EDGE_RENDER_SYSTEM;
}

void CameraComponent::onAttach()
{
    Actor *parent = getParent();
    if (!parent)
        return;

    if (_mainCamera)
    {
        parent->getScene()->setCamera(_camera);
    }

#ifndef NDEBUG
    AssetsManager *assetsManager = parent->getScene()->getAssetsManager();
    if (assetsManager)
    {
        _model = assetsManager->getModel("cmx_camera");
    }
#endif
}

void CameraComponent::update(float dt)
{
    Transform absoluteTransform = getAbsoluteTransform();
    _camera->setViewDirection(absoluteTransform.position, absoluteTransform.forward(), absoluteTransform.up());
}

void CameraComponent::render(const FrameInfo &frameInfo, VkPipelineLayout pipelineLayout)
{
#ifndef NDEBUG
    if (!CmxEditor::isActive())
    {
        return;
    }
#else
    return;
#endif
    if (getParent() == nullptr)
    {
        spdlog::critical("MeshComponent: _parent is expired");
        return;
    }

    if (_model)
    {
        EdgePushConstantData push{};
        push.modelMatrix = getAbsoluteTransform().mat4();
        push.color = _mainCamera ? glm::vec3{0.f, 1.f, 0.f} : glm::vec3{1.f, 1.f, 1.f};
        vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout,
                           VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(EdgePushConstantData),
                           &push);

        _model->bind(frameInfo.commandBuffer);
        _model->draw(frameInfo.commandBuffer);
    }
}

void CameraComponent::editor(int i)
{
    ImGui::Checkbox("make main", &_mainCamera);

    _camera->editor();
    Component::editor(i);
}

void CameraComponent::load(tinyxml2::XMLElement *componentElement)
{
    Component::load(componentElement);

    _camera->setFOV(componentElement->FloatAttribute("fov"));
    _camera->setNearPlane(componentElement->FloatAttribute("nearPlane"));
    _camera->setFarPlane(componentElement->FloatAttribute("farPlane"));
    _mainCamera = componentElement->BoolAttribute("isMain");

    onAttach();
}

tinyxml2::XMLElement &CameraComponent::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentComponent)
{
    tinyxml2::XMLElement &cameraComponent = Component::save(doc, parentComponent);
    cameraComponent.SetAttribute("fov", _camera->getFOV());
    cameraComponent.SetAttribute("nearPlane", _camera->getNearPlane());
    cameraComponent.SetAttribute("farPlane", _camera->getFarPlane());
    cameraComponent.SetAttribute("isMain", _mainCamera);

    parentComponent->InsertEndChild(&cameraComponent);

    return cameraComponent;
}

} // namespace cmx
