#include "cmx_hud_material.h"

// cmx
#include "cmx_camera.h"
#include "cmx_debug_util.h"
#include "cmx_frame_info.h"
#include "cmx_pipeline.h"
#include "cmx_render_system.h"
#include "cmx_renderer.h"

// lib
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace cmx
{

void HudMaterial::bind(FrameInfo *frameInfo, const Drawable *drawable)
{
    if (_boundID != _id)
    {
        _pipeline->bind(frameInfo->commandBuffer);

        _boundID = _id;
    }
}

void HudMaterial::editor()
{
}

void HudMaterial::initialize()
{
    RenderSystem *renderSystem = RenderSystem::getInstance();

    loadBindings();

    createPipelineLayout({});
    createPipeline(renderSystem->getGBuffer()->getRenderPass());
}

void HudMaterial::createPipelineLayout(std::vector<vk::DescriptorSetLayout> descriptorSetLayouts)
{
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = vk::StructureType::ePipelineLayoutCreateInfo;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = NULL;
    if (_renderSystem->getDevice()->device().createPipelineLayout(&pipelineLayoutInfo, nullptr, &_pipelineLayout) !=
        vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void HudMaterial::createPipeline(vk::RenderPass renderPass)
{
    assert(_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.bindingDescriptions.clear();
    pipelineConfig.attributeDescriptions.clear();
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = _pipelineLayout;

    pipelineConfig.depthStencilInfo.depthWriteEnable = vk::False;
    pipelineConfig.colorBlendAttachments[0].blendEnable = vk::True;
    pipelineConfig.colorBlendAttachments[0].srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
    pipelineConfig.colorBlendAttachments[0].dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
    pipelineConfig.colorBlendAttachments[1].blendEnable = vk::True;
    pipelineConfig.colorBlendAttachments[1].srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
    pipelineConfig.colorBlendAttachments[1].dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;

    _pipeline = std::make_unique<Pipeline>(*_renderSystem->getDevice(), _vertFilepath, _fragFilepath, pipelineConfig,
                                           "HUD material pipeline");
}

} // namespace cmx
