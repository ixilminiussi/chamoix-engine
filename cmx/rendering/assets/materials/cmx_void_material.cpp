#include "cmx_void_material.h"

// cmx
#include "cmx_drawable.h"
#include "cmx_frame_info.h"
#include "cmx_pipeline.h"
#include "cmx_render_system.h"
#include "cmx_renderer.h"

// lib
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace cmx
{

void VoidMaterial::bind(FrameInfo *frameInfo, const Drawable *drawable)
{
    if (_boundID != _id)
    {
        _pipeline->bind(frameInfo->commandBuffer);
        frameInfo->commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipelineLayout, 0, 1,
                                                    &frameInfo->globalDescriptorSet, 0, nullptr);

        _boundID = _id;
    }

    SimplePushConstantData push{};
    Transform transform = drawable->getWorldSpaceTransform();

    push.modelMatrix = transform.mat4();
    push.normalMatrix = transform.normalMatrix();

    frameInfo->commandBuffer.pushConstants(_pipelineLayout,
                                           vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0,
                                           sizeof(SimplePushConstantData), &push);
}

void VoidMaterial::editor()
{
    Material::editor();
}

tinyxml2::XMLElement *VoidMaterial::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const
{
    tinyxml2::XMLElement *materialElement = Material::save(doc, parentElement);

    return materialElement;
}

void VoidMaterial::load(tinyxml2::XMLElement *materialElement)
{
    Material::load(materialElement);
}

void VoidMaterial::initialize()
{
    RenderSystem *renderSystem = RenderSystem::getInstance();

    loadBindings();

    createPipelineLayout({renderSystem->getGlobalSetLayout()});
    createPipeline(renderSystem->getRenderer()->getSwapChainRenderPass());
}

void VoidMaterial::initialize(vk::RenderPass renderPass)
{
    RenderSystem *renderSystem = RenderSystem::getInstance();

    loadBindings();

    createPipelineLayout({renderSystem->getGlobalSetLayout()});
    createPipeline(renderPass);
}

void VoidMaterial::createPipelineLayout(std::vector<vk::DescriptorSetLayout> descriptorSetLayouts)
{
    vk::PushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);

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

void VoidMaterial::createPipeline(vk::RenderPass renderPass)
{
    assert(_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = _pipelineLayout;
    _pipeline = std::make_unique<Pipeline>(*_renderSystem->getDevice(), _vertFilepath, _fragFilepath, pipelineConfig);
}

} // namespace cmx
