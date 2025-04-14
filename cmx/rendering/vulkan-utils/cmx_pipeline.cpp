#include "cmx_pipeline.h"

// cmx
#include "cmx_debug_util.h"
#include "cmx_model.h"

// lib
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>

// std
#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vulkan/vulkan_enums.hpp>

namespace cmx
{

Pipeline::Pipeline(Device &device, const std::string &vertFilepath, const std::string &fragFilepath,
                   const PipelineConfigInfo &configInfo, const std::string &debugName)
    : _device{device}
{
    createGraphicsPipeline(vertFilepath, fragFilepath, configInfo);
    DebugUtil::nameObject(_graphicsPipeline, vk::ObjectType::ePipeline, debugName);
}

Pipeline::~Pipeline()
{
    if (!_freed)
    {
        spdlog::error("Pipeline: forgot to free before deletion");
    }
}

void Pipeline::free()
{
    _device.device().destroyShaderModule(_vertShaderModule);
    _device.device().destroyShaderModule(_fragShaderModule);
    _device.device().destroyPipeline(_graphicsPipeline);

    _freed = true;
}

void Pipeline::bind(vk::CommandBuffer commandBuffer)
{
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _graphicsPipeline);
}

std::vector<char> Pipeline::readFile(const std::string &filepath)
{
    std::ifstream file{filepath, std::ios::ate | std::ios::binary};

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file: " + filepath);
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();
    return buffer;
}

void Pipeline::createGraphicsPipeline(const std::string &vertFilepath, const std::string &fragFilepath,
                                      const PipelineConfigInfo &configInfo)
{
    assert(configInfo.pipelineLayout != VK_NULL_HANDLE &&
           "Cannot create graphics pipeline: no pipelineLayout provided in configInfo");
    assert(configInfo.renderPass != VK_NULL_HANDLE &&
           "Cannot create graphics pipeline: no renderPass provided in configInfo");

    std::vector<char> vertCode = readFile(vertFilepath);
    std::vector<char> fragCode = readFile(fragFilepath);

    createShaderModule(vertCode, &_vertShaderModule);
    createShaderModule(fragCode, &_fragShaderModule);

    vk::PipelineShaderStageCreateInfo shaderStages[2];
    shaderStages[0].sType = vk::StructureType::ePipelineShaderStageCreateInfo;
    shaderStages[0].stage = vk::ShaderStageFlagBits::eVertex;
    shaderStages[0].module = _vertShaderModule;
    shaderStages[0].pName = "main";
    shaderStages[0].flags = vk::PipelineShaderStageCreateFlagBits{};
    shaderStages[0].pNext = nullptr;
    shaderStages[0].pSpecializationInfo = nullptr;
    shaderStages[1].sType = vk::StructureType::ePipelineShaderStageCreateInfo;
    shaderStages[1].stage = vk::ShaderStageFlagBits::eFragment;
    shaderStages[1].module = _fragShaderModule;
    shaderStages[1].pName = "main";
    shaderStages[1].flags = vk::PipelineShaderStageCreateFlagBits{};
    shaderStages[1].pNext = nullptr;
    shaderStages[1].pSpecializationInfo = nullptr;

    const std::vector<vk::VertexInputBindingDescription> &bindingDescriptions = configInfo.bindingDescriptions;
    const std::vector<vk::VertexInputAttributeDescription> &attributeDescriptions = configInfo.attributeDescriptions;

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = vk::StructureType::ePipelineVertexInputStateCreateInfo;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
    vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

    vk::GraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = vk::StructureType::eGraphicsPipelineCreateInfo;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
    pipelineInfo.pViewportState = &configInfo.viewportInfo;
    pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
    pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
    pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
    pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
    pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;

    pipelineInfo.layout = configInfo.pipelineLayout;
    pipelineInfo.renderPass = configInfo.renderPass;
    pipelineInfo.subpass = configInfo.subpass;

    pipelineInfo.basePipelineIndex = -1;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (_device.device().createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicsPipeline) !=
        vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to create graphics pipeline");
    }
}

void Pipeline::createShaderModule(const std::vector<char> &code, vk::ShaderModule *shaderModule)
{
    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.sType = vk::StructureType::eShaderModuleCreateInfo;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    if (_device.device().createShaderModule(&createInfo, nullptr, shaderModule) != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to create shader module");
    }
}

void Pipeline::defaultPipelineConfigInfo(PipelineConfigInfo &configInfo)
{
    configInfo.inputAssemblyInfo.sType = vk::StructureType::ePipelineInputAssemblyStateCreateInfo;
    configInfo.inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
    configInfo.inputAssemblyInfo.primitiveRestartEnable = vk::False;

    configInfo.viewportInfo.sType = vk::StructureType::ePipelineViewportStateCreateInfo;
    configInfo.viewportInfo.viewportCount = 1;
    configInfo.viewportInfo.pViewports = nullptr;
    configInfo.viewportInfo.scissorCount = 1;
    configInfo.viewportInfo.pScissors = nullptr;

    configInfo.rasterizationInfo.sType = vk::StructureType::ePipelineRasterizationStateCreateInfo;
    configInfo.rasterizationInfo.depthClampEnable = vk::False;
    configInfo.rasterizationInfo.rasterizerDiscardEnable = vk::False;
    configInfo.rasterizationInfo.polygonMode = vk::PolygonMode::eFill;
    configInfo.rasterizationInfo.lineWidth = 1.0f;
    configInfo.rasterizationInfo.cullMode = vk::CullModeFlagBits::eFront;
    configInfo.rasterizationInfo.frontFace = vk::FrontFace::eClockwise;
    configInfo.rasterizationInfo.depthBiasEnable = vk::False;
    configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f; // Optional
    configInfo.rasterizationInfo.depthBiasClamp = 0.0f;          // Optional
    configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;    // Optional

    configInfo.multisampleInfo.sType = vk::StructureType::ePipelineMultisampleStateCreateInfo;
    configInfo.multisampleInfo.sampleShadingEnable = vk::False;
    configInfo.multisampleInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
    configInfo.multisampleInfo.minSampleShading = 1.0f;           // Optional
    configInfo.multisampleInfo.pSampleMask = nullptr;             // Optional
    configInfo.multisampleInfo.alphaToCoverageEnable = vk::False; // Optional
    configInfo.multisampleInfo.alphaToOneEnable = vk::False;      // Optional

    vk::PipelineColorBlendAttachmentState colorColorBlendAttachment;
    colorColorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                               vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    colorColorBlendAttachment.blendEnable = vk::False;
    colorColorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne;  // Optional
    colorColorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero; // Optional
    colorColorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;             // Optional
    colorColorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;  // Optional
    colorColorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero; // Optional
    colorColorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;             // Optional

    vk::PipelineColorBlendAttachmentState normalColorBlendAttachment;
    normalColorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                                vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    normalColorBlendAttachment.blendEnable = vk::False;
    normalColorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne;  // Optional
    normalColorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero; // Optional
    normalColorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;             // Optional
    normalColorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;  // Optional
    normalColorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero; // Optional
    normalColorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;             // Optional

    configInfo.colorBlendAttachments = {colorColorBlendAttachment, normalColorBlendAttachment};

    configInfo.colorBlendInfo.sType = vk::StructureType::ePipelineColorBlendStateCreateInfo;
    configInfo.colorBlendInfo.logicOpEnable = vk::False;
    configInfo.colorBlendInfo.logicOp = vk::LogicOp::eCopy; // Optional
    configInfo.colorBlendInfo.attachmentCount = static_cast<uint32_t>(configInfo.colorBlendAttachments.size());
    configInfo.colorBlendInfo.pAttachments = configInfo.colorBlendAttachments.data();
    configInfo.colorBlendInfo.blendConstants[0] = 0.0f; // Optional
    configInfo.colorBlendInfo.blendConstants[1] = 0.0f; // Optional
    configInfo.colorBlendInfo.blendConstants[2] = 0.0f; // Optional
    configInfo.colorBlendInfo.blendConstants[3] = 0.0f; // Optional

    configInfo.depthStencilInfo.sType = vk::StructureType::ePipelineDepthStencilStateCreateInfo;
    configInfo.depthStencilInfo.depthTestEnable = vk::True;
    configInfo.depthStencilInfo.depthWriteEnable = vk::True;
    configInfo.depthStencilInfo.depthCompareOp = vk::CompareOp::eLess;
    configInfo.depthStencilInfo.depthBoundsTestEnable = vk::False;
    configInfo.depthStencilInfo.minDepthBounds = 0.0f; // Optional
    configInfo.depthStencilInfo.maxDepthBounds = 1.0f; // Optional
    configInfo.depthStencilInfo.stencilTestEnable = vk::False;
    configInfo.depthStencilInfo.front = vk::StencilOpState{}; // Optional
    configInfo.depthStencilInfo.back = vk::StencilOpState{};  // Optional

    configInfo.dynamicStateEnables = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    configInfo.dynamicStateInfo.sType = vk::StructureType::ePipelineDynamicStateCreateInfo;
    configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data();
    configInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
    configInfo.dynamicStateInfo.flags = vk::PipelineDynamicStateCreateFlagBits{};

    configInfo.bindingDescriptions = Model::Vertex::getBindingDescriptions();
    configInfo.attributeDescriptions = Model::Vertex::getAttributeDescriptions();
}

} // namespace cmx
