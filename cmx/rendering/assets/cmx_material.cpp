#include "cmx_material.h"
#include "cmx_frame_info.h"
#include "cmx_pipeline.h"
#include "cmx_render_system.h"
#include "cmx_renderer.h"
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace cmx
{

size_t Material::_idProvider{0};
size_t Material::_boundID{1};

Material::Material(std::string vertPath, std::string fragPath)
    : _vertFilepath{vertPath}, _fragFilepath{fragPath}, _id{_idProvider}
{
    _renderSystem = RenderSystem::getInstance();

    _idProvider += 1;
    _boundID = _idProvider + 1;
}

void Material::bind(FrameInfo *frameInfo)
{
    if (_boundID != _id)
    {
        _pipeline->bind(frameInfo->commandBuffer);

        _boundID = _id;
    }
}

void Material::editor()
{
}

void Material::resetBoundID()
{
    _boundID = _idProvider + 1;
}

Material::Material(int ID) : _id(ID)
{
}

void Material::initialize()
{
    RenderSystem *renderer = RenderSystem::getInstance();

    createPipelineLayout({});
    createPipeline(renderer->getRenderer()->getSwapChainRenderPass());
}

void Material::createPipelineLayout(std::vector<vk::DescriptorSetLayout> descriptorSetLayouts)
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

void Material::createPipeline(vk::RenderPass renderPass)
{
    assert(_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = _pipelineLayout;
    _pipeline = std::make_unique<Pipeline>(*_renderSystem->getDevice(), _vertFilepath, _fragFilepath, pipelineConfig);
}

} // namespace cmx
