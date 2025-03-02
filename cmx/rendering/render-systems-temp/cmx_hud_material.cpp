#include "cmx_hud_material.h"

namespace cmx
{

void HudMaterialSystem::initialize()
{
    initializeUbo();

    _globalPool = DescriptorPool::Builder(*_device.get())
                      .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
                      .addPoolSize(vk::DescriptorType::eUniformBuffer, SwapChain::MAX_FRAMES_IN_FLIGHT)
                      .build();

    auto globalSetLayout = DescriptorSetLayout::Builder(*_device.get())
                               .addBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eAllGraphics)
                               .build();

    for (int i = 0; i < _globalDescriptorSets.size(); i++)
    {
        auto bufferInfo = _uboBuffers[i]->descriptorInfo();
        DescriptorWriter(*globalSetLayout, *_globalPool).writeBuffer(0, &bufferInfo).build(_globalDescriptorSets[i]);
    }

    createPipelineLayout(
        {globalSetLayout->getDescriptorSetLayout(), _samplerDescriptorSetLayout->getDescriptorSetLayout()});
    createPipeline(_renderer->getSwapChainRenderPass());

    _dummyBuffer =
        std::make_unique<Buffer>(*_device, sizeof(float), 1, vk::BufferUsageFlagBits::eVertexBuffer,
                                 vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    spdlog::info("HudMaterialSystem: Successfully initialized!");
}

void HudMaterialSystem::free()
{
    RenderSystem::free();
    _dummyBuffer->free();
}

void HudMaterialSystem::render(const FrameInfo *frameInfo, std::vector<std::shared_ptr<Component>> &renderQueue,
                               class GraphicsManager *graphicsManager)
{
    if (!_visible)
        return;

    _pipeline->bind(frameInfo->commandBuffer);

    frameInfo->commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipelineLayout, 0, 1,
                                                &frameInfo->globalDescriptorSet, 0, nullptr);

    _activeSystem = HUD_RENDER_SYSTEM;

    vk::DeviceSize offsets[] = {0};
    vk::Buffer buffer = _dummyBuffer->getBuffer();
    frameInfo->commandBuffer.bindVertexBuffers(0, 1, &buffer, offsets);

    std::sort(renderQueue.begin(), renderQueue.end(),
              [](const std::shared_ptr<Component> a, const std::shared_ptr<Component> b) {
                  return a->getRenderZ() < b->getRenderZ();
              });

    auto it = renderQueue.begin();
    while (it != renderQueue.end())
    {
        auto renderComponent = *it;

        if (renderComponent->getRequestedRenderSystem() != HUD_RENDER_SYSTEM)
        {
            it = renderQueue.erase(it);
            graphicsManager->addToQueue(renderComponent);
            continue;
        }
        if (renderComponent->getVisible())
        {
            renderComponent->render(*frameInfo, _pipelineLayout);
        }
        it++;
    }
}

void HudMaterialSystem::createPipelineLayout(std::vector<vk::DescriptorSetLayout> descriptorSetLayouts)
{
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = vk::StructureType::ePipelineLayoutCreateInfo;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = NULL;
    if (_device->device().createPipelineLayout(&pipelineLayoutInfo, nullptr, &_pipelineLayout) != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void HudMaterialSystem::createPipeline(vk::RenderPass renderPass)
{
    assert(_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.bindingDescriptions.clear();
    pipelineConfig.attributeDescriptions.clear();
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = _pipelineLayout;

    pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
    pipelineConfig.colorBlendAttachment.blendEnable = VK_TRUE;
    pipelineConfig.colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
    pipelineConfig.colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;

    _pipeline =
        std::make_unique<Pipeline>(*_device.get(), "shaders/hud.vert.spv", "shaders/hud.frag.spv", pipelineConfig);
}

void HudMaterialSystem::editor(int i)
{
    ImGui::PushID(i);
    ImGui::Checkbox("Hud##", &_visible);
    ImGui::PopID();
}

} // namespace cmx
