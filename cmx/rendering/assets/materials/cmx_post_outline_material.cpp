#include "cmx_post_outline_material.h"

// cmx
#include "cmx_drawable.h"
#include "cmx_frame_info.h"
#include "cmx_graphics_manager.h"
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
}

void PostOutlineMaterial::editor()
{
    Material::editor();
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
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = vk::StructureType::ePipelineLayoutCreateInfo;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

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

    pipelineConfig.colorBlendAttachments.erase(pipelineConfig.colorBlendAttachments.end());
    pipelineConfig.colorBlendInfo.attachmentCount = 1;
    pipelineConfig.bindingDescriptions.clear();
    pipelineConfig.attributeDescriptions.clear();

    _pipeline = std::make_unique<Pipeline>(*_renderSystem->getDevice(), _vertFilepath, _fragFilepath, pipelineConfig,
                                           "post outline material pipeline");
}

} // namespace cmx
