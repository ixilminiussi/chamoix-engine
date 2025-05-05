#include "cmx_post_outline_material.h"

// cmx
#include "cmx_camera.h"
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

void PostOutlineMaterial::bind(FrameInfo *frameInfo, const Drawable *)
{
    if (_boundID != _id)
    {
        _pipeline->bind(frameInfo->commandBuffer);

        size_t *descriptorSetIDs = _renderSystem->getSamplerDescriptorSetIDs();
        frameInfo->commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipelineLayout, 0, 1,
                                                    &(_renderSystem->getSamplerDescriptorSet(descriptorSetIDs[0])), 0,
                                                    nullptr);
        frameInfo->commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipelineLayout, 1, 1,
                                                    &(_renderSystem->getSamplerDescriptorSet(descriptorSetIDs[1])), 0,
                                                    nullptr);
        frameInfo->commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipelineLayout, 2, 1,
                                                    &(_renderSystem->getSamplerDescriptorSet(descriptorSetIDs[2])), 0,
                                                    nullptr);

        _boundID = _id;
    }

    PushConstantData push{};
    push.edgeColor = _edgeColor;
    push.colorEdgeThickness = _colorEdgeThickness;
    push.colorEdgeThreshold = _colorEdgeThreshold;
    push.colorDepthFactor = _colorDepthFactor;
    push.normalEdgeThickness = _normalEdgeThickness;
    push.normalEdgeThreshold = _normalEdgeThreshold;
    push.normalDepthFactor = _normalDepthFactor;
    push.depthEdgeThickness = _depthEdgeThickness;
    push.depthEdgeThreshold = _depthEdgeThreshold;
    push.depthDepthFactor = _depthDepthFactor;
    push.nearPlane = frameInfo->camera->getNearPlane();
    push.farPlane = frameInfo->camera->getFarPlane();

    frameInfo->commandBuffer.pushConstants(_pipelineLayout,
                                           vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0,
                                           sizeof(PushConstantData), &push);
}

void PostOutlineMaterial::editor()
{
    Material::editor();

    ImGui::ColorEdit3("Edge Color##Outline", (float *)&_edgeColor);

    if (ImGui::TreeNode("Color"))
    {
        ImGui::DragFloat("Line thickness##Color", &_colorEdgeThickness, 1.f, 0.f, 20.f);
        ImGui::DragFloat("Edge threshold##Color", &_colorEdgeThreshold, 0.05f, 0.f, 1.f);
        ImGui::DragFloat("Depth factor##Color", &_colorDepthFactor, 0.05f, 0.f, 5.f);
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Normal"))
    {
        ImGui::DragFloat("Line thickness##Normal", &_normalEdgeThickness, 1.f, 0.f, 20.f);
        ImGui::DragFloat("Edge threshold##Normal", &_normalEdgeThreshold, 0.01f, 0.f, 1.f);
        ImGui::DragFloat("Depth factor##Depth", &_normalDepthFactor, 0.01f, 0.01f, .5f);
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Depth"))
    {
        ImGui::DragFloat("Line thickness##Depth", &_depthEdgeThickness, 1.f, 0.f, 20.f);
        ImGui::DragFloat("Edge threshold##Depth", &_depthEdgeThreshold, 0.05f, 0.f, 10.f);
        ImGui::DragFloat("Depth factor##Depth", &_depthDepthFactor, 0.01f, 0.01f, .5f);
        ImGui::TreePop();
    }
}

tinyxml2::XMLElement *PostOutlineMaterial::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const
{
    return Material::save(doc, parentElement);
}

void PostOutlineMaterial::load(tinyxml2::XMLElement *materialElement)
{
    Material::load(materialElement);
}

void PostOutlineMaterial::initialize()
{
    RenderSystem *renderSystem = RenderSystem::getInstance();

    loadBindings();

    createPipelineLayout({renderSystem->getSamplerDescriptorSetLayout(), renderSystem->getSamplerDescriptorSetLayout(),
                          renderSystem->getSamplerDescriptorSetLayout()});
    createPipeline(renderSystem->getRenderer()->getSwapChainRenderPass());
}

void PostOutlineMaterial::createPipelineLayout(std::vector<vk::DescriptorSetLayout> descriptorSetLayouts)
{
    vk::PushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PushConstantData);

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

    _requestedSamplerCount -= 1;
}

void PostOutlineMaterial::createPipeline(vk::RenderPass renderPass)
{
    assert(_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = _pipelineLayout;

    pipelineConfig.colorBlendInfo.attachmentCount = 1;
    pipelineConfig.bindingDescriptions.clear();
    pipelineConfig.attributeDescriptions.clear();

    _pipeline = std::make_unique<Pipeline>(*_renderSystem->getDevice(), _vertFilepath, _fragFilepath, pipelineConfig,
                                           "post outline material pipeline");
}

} // namespace cmx
