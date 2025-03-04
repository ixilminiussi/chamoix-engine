#include "cmx_shaded_material.h"

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

void ShadedMaterial::bind(FrameInfo *frameInfo, const Drawable *drawable)
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
    push.normalMatrix[3] = glm::vec4(_color, 1.0f);

    push.normalMatrix[0][3] = _UVoffset.x;
    push.normalMatrix[1][3] = _UVoffset.y;
    push.normalMatrix[2][3] = _worldSpaceUV ? _UVScale : 0.f;
    push.normalMatrix[3][3] = _textured ? glm::radians(_UVRotate) : 100.f;

    frameInfo->commandBuffer.pushConstants(_pipelineLayout,
                                           vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0,
                                           sizeof(SimplePushConstantData), &push);
}

void ShadedMaterial::editor()
{
    ImGui::Checkbox("use texture", &_textured);

    if (_textured)
    {
        ImGui::Checkbox("World space UV", &_worldSpaceUV);
        if (_worldSpaceUV)
        {
            ImGui::SliderFloat2("UV offset", (float *)&_UVoffset, -1.f, 1.f);
            ImGui::DragFloat("Scale", &_UVScale);
            ImGui::DragFloat("Rotate", &_UVRotate, 1.f, -180.f, 180.f);
        }
    }

    ImGui::ColorEdit3("Color##", (float *)&_color);
}

void ShadedMaterial::initialize()
{
    RenderSystem *renderSystem = RenderSystem::getInstance();

    loadBindings();

    createPipelineLayout({renderSystem->getGlobalSetLayout(), renderSystem->getSamplerDescriptorSetLayout()});
    createPipeline(renderSystem->getRenderer()->getSwapChainRenderPass());
}

void ShadedMaterial::createPipelineLayout(std::vector<vk::DescriptorSetLayout> descriptorSetLayouts)
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

void ShadedMaterial::createPipeline(vk::RenderPass renderPass)
{
    assert(_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = _pipelineLayout;
    _pipeline = std::make_unique<Pipeline>(*_renderSystem->getDevice(), _vertFilepath, _fragFilepath, pipelineConfig);
}

} // namespace cmx
