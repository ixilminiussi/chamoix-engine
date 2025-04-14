#ifndef CMX_PIPELINE
#define CMX_PIPELINE

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

    std::vector<vk::VertexInputBindingDescription> bindingDescriptions{};
    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions{};
    vk::PipelineViewportStateCreateInfo viewportInfo;
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    vk::PipelineRasterizationStateCreateInfo rasterizationInfo;
    vk::PipelineMultisampleStateCreateInfo multisampleInfo;
    std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments;
    vk::PipelineColorBlendStateCreateInfo colorBlendInfo;
    vk::PipelineDepthStencilStateCreateInfo depthStencilInfo;
    std::vector<vk::DynamicState> dynamicStateEnables;
    vk::PipelineDynamicStateCreateInfo dynamicStateInfo;
    vk::PipelineLayout pipelineLayout = nullptr;
    vk::RenderPass renderPass = nullptr;
    uint32_t subpass = 0;
};

class Pipeline
{
  public:
    Pipeline(Device &, const std::string &vertFilepath, const std::string &fragFilepath, const PipelineConfigInfo &,
             const std::string &debugName);
    ~Pipeline();

    Pipeline(const Pipeline &) = delete;
    Pipeline &operator=(const Pipeline &) = delete;

    void bind(vk::CommandBuffer);

    void free();

    static void defaultPipelineConfigInfo(PipelineConfigInfo &);

  private:
    static std::vector<char> readFile(const std::string &filepath);

    void createGraphicsPipeline(const std::string &vertFilepath, const std::string &fragFilepath,
                                const PipelineConfigInfo &);

    void createShaderModule(const std::vector<char> &code, vk::ShaderModule *);

    Device &_device;
    vk::Pipeline _graphicsPipeline;
    vk::ShaderModule _vertShaderModule;
    vk::ShaderModule _fragShaderModule;

    bool _freed{false};
};
} // namespace cmx

#endif
