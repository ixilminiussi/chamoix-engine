#pragma once

#include "cmx_device.h"

// lib
#include <vulkan/vulkan_core.h>

// std
#include <string>
#include <vector>

namespace cmx
{

struct PipelineConfigInfo
{
    PipelineConfigInfo(const PipelineConfigInfo &) = delete;
    PipelineConfigInfo &operator=(const PipelineConfigInfo &) = delete;

    VkPipelineViewportStateCreateInfo viewportInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    std::vector<VkDynamicState> dynamicStateEnables;
    VkPipelineDynamicStateCreateInfo dynamicStateInfo;
    VkPipelineLayout pipelineLayout = nullptr;
    VkRenderPass renderPass = nullptr;
    uint32_t subpass = 0;
};

class CmxPipeline
{
  public:
    CmxPipeline(CmxDevice &, const std::string &vertFilepath, const std::string &fragFilepath,
                const PipelineConfigInfo &);
    ~CmxPipeline();

    CmxPipeline(const CmxPipeline &) = delete;
    CmxPipeline &operator=(const CmxPipeline &) = delete;

    void bind(VkCommandBuffer);

    static void defaultPipelineConfigInfo(PipelineConfigInfo &);

  private:
    static std::vector<char> readFile(const std::string &filepath);

    void createGraphicsPipeline(const std::string &vertFilepath, const std::string &fragFilepath,
                                const PipelineConfigInfo &);

    void createShaderModule(const std::vector<char> &code, VkShaderModule *);

    CmxDevice &_cmxDevice;
    VkPipeline _graphicsPipeline;
    VkShaderModule _vertShaderModule;
    VkShaderModule _fragShaderModule;
};
} // namespace cmx
