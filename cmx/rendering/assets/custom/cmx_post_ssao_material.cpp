#include "cmx_post_ssao_material.h"

// cmx
#include "cmx_camera.h"
#include "cmx_drawable.h"
#include "cmx_frame_info.h"
#include "cmx_game.h"
#include "cmx_pipeline.h"
#include "cmx_render_pass.h"
#include "cmx_render_system.h"
#include "cmx_texture.h"

// lib
#include <glm/matrix.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace cmx
{

void PostSSAOMaterial::bind(FrameInfo *frameInfo, const Drawable *)
{
    if (_boundID != _id)
    {
        _pipeline->bind(frameInfo->commandBuffer);

        const std::vector<RenderTarget> &renderTargets = _renderSystem->getGBuffer()->getRenderTargets();
        frameInfo->commandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics, _pipelineLayout, 0, 1,
            &(_renderSystem->getSamplerDescriptorSet(renderTargets[0].descriptorSetID)), 0, nullptr);
        frameInfo->commandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics, _pipelineLayout, 1, 1,
            &(_renderSystem->getSamplerDescriptorSet(renderTargets[1].descriptorSetID)), 0, nullptr);
        frameInfo->commandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics, _pipelineLayout, 2, 1,
            &(_renderSystem->getSamplerDescriptorSet(renderTargets[2].descriptorSetID)), 0, nullptr);
        frameInfo->commandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics, _pipelineLayout, 3, 1,
            &(_renderSystem->getSamplerDescriptorSet(renderTargets[3].descriptorSetID)), 0, nullptr);
        noiseTexture->bind(frameInfo->commandBuffer, _pipelineLayout, 4);

        _boundID = _id;
    }

    PushConstantData push{};
    push.projection = frameInfo->camera->getProjection();
    push.view = frameInfo->camera->getView();

    frameInfo->commandBuffer.pushConstants(_pipelineLayout,
                                           vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0,
                                           sizeof(PushConstantData), &push);
}

void PostSSAOMaterial::editor()
{
    Material::editor();
}

tinyxml2::XMLElement *PostSSAOMaterial::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const
{
    return Material::save(doc, parentElement);
}

void PostSSAOMaterial::load(tinyxml2::XMLElement *materialElement)
{
    Material::load(materialElement);
}

void PostSSAOMaterial::initialize()
{
    RenderSystem *renderSystem = RenderSystem::getInstance();

    loadBindings();

    createPipelineLayout({renderSystem->getSamplerDescriptorSetLayout(), renderSystem->getSamplerDescriptorSetLayout(),
                          renderSystem->getSamplerDescriptorSetLayout(), renderSystem->getSamplerDescriptorSetLayout(),
                          renderSystem->getSamplerDescriptorSetLayout()});

    noiseTexture = Texture::create2DTextureFromFile(RenderSystem::getInstance()->getDevice(),
                                                    ".cmx_assets/noise/noise.png", "noise");

    createPipeline(renderSystem->getSSAOBuffer()[0]->getRenderPass());
}

void PostSSAOMaterial::free()
{
    Material::free();

    noiseTexture->free();
    delete noiseTexture;
}

void PostSSAOMaterial::createPipelineLayout(std::vector<vk::DescriptorSetLayout> descriptorSetLayouts)
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

void PostSSAOMaterial::createPipeline(vk::RenderPass renderPass)
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
