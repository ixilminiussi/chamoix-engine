#include "cmx_camera_component.h"

// cmx
#include "cmx_actor.h"
#include "cmx_camera.h"
#include "cmx_register.h"
#ifndef NDEBUG
#include "cmx_mesh_material.h"
#endif

// lib
#include "imgui.h"
#include <vulkan/vulkan_enums.hpp>

namespace cmx
{

CameraComponent::CameraComponent()
    : _camera{std::make_shared<Camera>()}
#ifndef NDEBUG
      ,
      Drawable{&_parent}
#endif
{
}

void CameraComponent::onAttach()
{
    Actor *parent = getParent();
    if (!parent)
        return;

#ifndef NDEBUG
    AssetsManager *assetsManager = getScene()->getAssetsManager();
    _material = (MeshMaterial *)assetsManager->makeUnique("mesh_material");
    setDrawOption({
        (Material *)_material,
        assetsManager->getModel("cmx_camera"),
        {},
    });
#endif
}

void CameraComponent::update(float dt)
{
    Transform absoluteTransform = getWorldSpaceTransform();
    _camera->setViewDirection(absoluteTransform.position, absoluteTransform.forward(), absoluteTransform.up());
}

// void CameraComponent::render(const FrameInfo &frameInfo, vk::PipelineLayout pipelineLayout)
// {
// #ifndef NDEBUG
//     if (!Editor::isActive())
//     {
//         return;
//     }
//     if (getParent() == nullptr)
//     {
//         spdlog::critical("MeshComponent: _parent is expired");
//         return;
//     }
//
//     if (_model)
//     {
//         EdgePushConstantData push{};
//         push.modelMatrix = getWorldSpaceTransform().mat4();
//         push.color = _mainCamera ? glm::vec3{0.f, 1.f, 0.f} : glm::vec3{1.f, 1.f, 1.f};
//         frameInfo.commandBuffer.pushConstants(pipelineLayout,
//                                               vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
//                                               0, sizeof(EdgePushConstantData), &push);
//
//         _model->bind(frameInfo.commandBuffer);
//         _model->draw(frameInfo.commandBuffer);
//     }
// #else
//     return;
// #endif
// }

void CameraComponent::editor(int i)
{
#ifndef NDEBUG
    if (ImGui::Checkbox("make main", &_mainCamera))
    {
        if (_mainCamera)
        {
            _material->setColor({1, 1, 0});
        }
        else
        {
            _material->setColor({0, 1, 1});
        }
    }

    _camera->editor();
    Component::editor(i);
    Drawable::editor(i);
#endif
}

void CameraComponent::load(tinyxml2::XMLElement *componentElement)
{
    Component::load(componentElement);
#ifndef NDEBUG
    Drawable::load(componentElement);
#endif

    _camera->setFOV(componentElement->FloatAttribute("fov"));
    _camera->setNearPlane(componentElement->FloatAttribute("nearPlane"));
    _camera->setFarPlane(componentElement->FloatAttribute("farPlane"));
    _mainCamera = componentElement->BoolAttribute("isMain");

    if (_mainCamera)
    {
        _material->setColor({1, 1, 0});
    }
    else
    {
        _material->setColor({0, 1, 1});
    }

    if (_mainCamera)
    {
        _parent->getScene()->setCamera(_camera);
    }
}

tinyxml2::XMLElement &CameraComponent::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const
{
    tinyxml2::XMLElement &cameraComponent = Component::save(doc, parentElement);
#ifndef NDEBUG
    Drawable::save(doc, &cameraComponent);
#endif
    cameraComponent.SetAttribute("fov", _camera->getFOV());
    cameraComponent.SetAttribute("nearPlane", _camera->getNearPlane());
    cameraComponent.SetAttribute("farPlane", _camera->getFarPlane());
    cameraComponent.SetAttribute("isMain", _mainCamera);

    parentElement->InsertEndChild(&cameraComponent);

    return cameraComponent;
}

} // namespace cmx
