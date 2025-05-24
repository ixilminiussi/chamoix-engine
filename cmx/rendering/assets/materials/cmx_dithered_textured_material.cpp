#include "cmx_dithered_textured_material.h"

// cmx
#include "cmx_drawable.h"
#include "cmx_frame_info.h"
#include "cmx_graphics_manager.h"
#include "cmx_pipeline.h"
#include "cmx_render_pass.h"
#include "cmx_render_system.h"

// lib
#include <imgui.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace cmx
{

void DitheredTexturedMaterial::bind(FrameInfo *frameInfo, const Drawable *drawable)
{
    if (_boundID != _id)
    {
        _pipeline->bind(frameInfo->commandBuffer);

        frameInfo->commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipelineLayout, 0, 1,
                                                    &frameInfo->globalDescriptorSet, 0, nullptr);

        const std::vector<size_t> &descriptorSetIDs = GraphicsManager::getDescriptorSetIDs();
        frameInfo->commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipelineLayout, 3, 1,
                                                    &(_renderSystem->getSamplerDescriptorSet(descriptorSetIDs[0])), 0,
                                                    nullptr);

        _boundID = _id;
    }

    PushConstantData push{};
    Transform transform = drawable->getWorldSpaceTransform();

    push.modelMatrix = transform.mat4();
    push.normalMatrix = transform.normalMatrix();

    push.normalMatrix[0][3] = _UVoffset.x;
    push.normalMatrix[1][3] = _UVoffset.y;
    push.normalMatrix[2][3] = _worldSpaceUV ? _UVScale : 0.f;
    push.normalMatrix[3][3] = glm::radians(_UVRotate);

    push.normalMatrix[3][0] = _scale;
    push.normalMatrix[3][1] = _threshold;
    push.normalMatrix[3][2] = _mSpacing;

    frameInfo->commandBuffer.pushConstants(_pipelineLayout,
                                           vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0,
                                           sizeof(PushConstantData), &push);
}

void DitheredTexturedMaterial::editor()
{
    Material::editor();

    ImGui::Checkbox("World space UV", &_worldSpaceUV);
    if (_worldSpaceUV)
    {
        ImGui::SliderFloat2("UV offset", (float *)&_UVoffset, -1.f, 1.f);
        ImGui::DragFloat("UV Scale##1", &_UVScale);
        ImGui::DragFloat("UV Rotate", &_UVRotate, 1.f, -180.f, 180.f);
    }

    ImGui::Separator();

    ImGui::DragFloat("Scale##2", &_scale, 0.1f, 1.f, 8.f);
    ImGui::SliderFloat("Threshold", &_threshold, 0.5f, 2.f);
    ImGui::SliderFloat("M", &_mSpacing, 0.f, 1.f);
}

tinyxml2::XMLElement *DitheredTexturedMaterial::save(tinyxml2::XMLDocument &doc,
                                                     tinyxml2::XMLElement *parentElement) const
{
    tinyxml2::XMLElement *materialElement = Material::save(doc, parentElement);
    if (materialElement == nullptr)
        return nullptr;

    materialElement->SetAttribute("worldSpaceUV", _worldSpaceUV);
    if (_worldSpaceUV)
    {
        materialElement->SetAttribute("UVoffsetX", _UVoffset.x);
        materialElement->SetAttribute("UVoffsetY", _UVoffset.y);
        materialElement->SetAttribute("UVscale", _UVScale);
        materialElement->SetAttribute("UVrotate", _UVRotate);
    }

    // dot
    materialElement->SetAttribute("scale", _scale);
    materialElement->SetAttribute("threshold", _threshold);
    materialElement->SetAttribute("m", _mSpacing);

    return materialElement;
}

void DitheredTexturedMaterial::load(tinyxml2::XMLElement *materialElement)
{
    Material::load(materialElement);

    _worldSpaceUV = materialElement->BoolAttribute("worldSpaceUV");
    if (_worldSpaceUV)
    {
        _UVoffset =
            glm::vec2{materialElement->FloatAttribute("UVoffsetX"), materialElement->FloatAttribute("UVoffsetY")};
        _UVScale = materialElement->FloatAttribute("UVscale");
        _UVRotate = materialElement->FloatAttribute("UVrotate");
    }

    // dots
    _scale = materialElement->FloatAttribute("scale", 5.f);
    _threshold = materialElement->FloatAttribute("threshold", 1.f);
    _mSpacing = materialElement->FloatAttribute("m", .2f);
}

void DitheredTexturedMaterial::initialize()
{
    RenderSystem *renderSystem = RenderSystem::getInstance();

    loadBindings();

    createPipelineLayout({renderSystem->getGlobalSetLayout(), renderSystem->getSamplerDescriptorSetLayout(),
                          renderSystem->getSamplerDescriptorSetLayout(),
                          renderSystem->getSamplerDescriptorSetLayout()});
    createPipeline(renderSystem->getGBuffer()->getRenderPass());
}

void DitheredTexturedMaterial::createPipelineLayout(std::vector<vk::DescriptorSetLayout> descriptorSetLayouts)
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

void DitheredTexturedMaterial::createPipeline(vk::RenderPass renderPass)
{
    assert(_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = _pipelineLayout;
    _pipeline = std::make_unique<Pipeline>(*_renderSystem->getDevice(), _vertFilepath, _fragFilepath, pipelineConfig,
                                           "dithered textured material pipeline");
}

} // namespace cmx
