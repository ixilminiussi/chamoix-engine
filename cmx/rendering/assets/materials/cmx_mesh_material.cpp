#include "cmx_mesh_material.h"

// cmx
#include "cmx_drawable.h"
#include "cmx_frame_info.h"
#include "cmx_pipeline.h"
#include "cmx_render_system.h"
#include "cmx_renderer.h"
#include "imgui.h"

// lib
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace cmx
{

void MeshMaterial::bind(FrameInfo *frameInfo, const Drawable *drawable)
{
    if (_boundID != _id)
    {
        _pipeline->bind(frameInfo->commandBuffer);
        frameInfo->commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipelineLayout, 0, 1,
                                                    &frameInfo->globalDescriptorSet, 0, nullptr);

        _boundID = _id;
    }

    EdgePushConstantData push{};
    push.modelMatrix = drawable->getWorldSpaceTransform().mat4();
    push.color = _color;

    frameInfo->commandBuffer.pushConstants(_pipelineLayout,
                                           vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0,
                                           sizeof(EdgePushConstantData), &push);
}

void MeshMaterial::editor()
{
    Material::editor();

    ImGui::ColorPicker3("color", (float *)&_color);
}

tinyxml2::XMLElement *MeshMaterial::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const
{
    tinyxml2::XMLElement *materialElement = Material::save(doc, parentElement);
    if (materialElement == nullptr)
        return materialElement;

    materialElement->SetAttribute("r", _color.r);
    materialElement->SetAttribute("g", _color.g);
    materialElement->SetAttribute("b", _color.b);

    return materialElement;
}

void MeshMaterial::load(tinyxml2::XMLElement *materialElement)
{
    Material::load(materialElement);

    _color.r = materialElement->FloatAttribute("r");
    _color.g = materialElement->FloatAttribute("g");
    _color.b = materialElement->FloatAttribute("b");
}

void MeshMaterial::initialize()
{
    RenderSystem *renderSystem = RenderSystem::getInstance();

    loadBindings();

    createPipelineLayout({renderSystem->getGlobalSetLayout()});
    createPipeline(renderSystem->getRenderer()->getSwapChainRenderPass());
}

void MeshMaterial::createPipelineLayout(std::vector<vk::DescriptorSetLayout> descriptorSetLayouts)
{
    vk::PushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(EdgePushConstantData);

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = vk::StructureType::ePipelineLayoutCreateInfo;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (_renderSystem->getDevice()->device().createPipelineLayout(&pipelineLayoutInfo, nullptr, &_pipelineLayout) !=
        vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void MeshMaterial::createPipeline(vk::RenderPass renderPass)
{
    assert(_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = _pipelineLayout;
    pipelineConfig.rasterizationInfo.polygonMode = vk::PolygonMode::eLine;
    pipelineConfig.rasterizationInfo.lineWidth = 1.0f;
    pipelineConfig.rasterizationInfo.cullMode = vk::CullModeFlagBits::eNone;

    _pipeline = std::make_unique<Pipeline>(*_renderSystem->getDevice(), _vertFilepath, _fragFilepath, pipelineConfig);
}

} // namespace cmx
