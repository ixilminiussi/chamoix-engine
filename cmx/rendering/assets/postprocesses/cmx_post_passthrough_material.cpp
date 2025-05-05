#include "cmx_post_passthrough_material.h"

// cmx
#include "cmx_camera.h"
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

void PostPassthroughMaterial::bind(FrameInfo *frameInfo, const Drawable *)
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

    PushConstantData push{};
    push.status = _status;
    push.nearPlane = frameInfo->camera->getNearPlane();
    push.farPlane = frameInfo->camera->getFarPlane();

    frameInfo->commandBuffer.pushConstants(_pipelineLayout,
                                           vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0,
                                           sizeof(PushConstantData), &push);
}

void PostPassthroughMaterial::editor()
{
    Material::editor();

    static std::map<int, std::string> options{{0, "albedo"}, {1, "normals"}, {2, "depth"}};

    const char *selected;

    if (options.find(_status) != options.end())
    {
        selected = options[_status].c_str();
    }
    else
    {
        _status = 0;
        selected = options[_status].c_str();
    }

    if (ImGui::BeginCombo("Visualizing", selected))
    {
        for (auto [status, name] : options)
        {
            bool isSelected = (strcmp(selected, name.c_str()) == 0);
            if (ImGui::Selectable(name.c_str(), isSelected) && !isSelected)
            {
                selected = name.c_str();
                _status = status;
                spdlog::info("{0}", _status);
                isSelected = true;
            }

            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }
}

tinyxml2::XMLElement *PostPassthroughMaterial::save(tinyxml2::XMLDocument &doc,
                                                    tinyxml2::XMLElement *parentElement) const
{
    tinyxml2::XMLElement *materialElement = Material::save(doc, parentElement);

    materialElement->SetAttribute("status", _status);

    return materialElement;
}

void PostPassthroughMaterial::load(tinyxml2::XMLElement *materialElement)
{
    Material::load(materialElement);

    _status = materialElement->IntAttribute("status", 0);
}

void PostPassthroughMaterial::initialize()
{
    RenderSystem *renderSystem = RenderSystem::getInstance();

    _status = 0;

    loadBindings();

    createPipelineLayout({renderSystem->getSamplerDescriptorSetLayout(), renderSystem->getSamplerDescriptorSetLayout(),
                          renderSystem->getSamplerDescriptorSetLayout()});
    createPipeline(renderSystem->getRenderer()->getSwapChainRenderPass());
}

void PostPassthroughMaterial::createPipelineLayout(std::vector<vk::DescriptorSetLayout> descriptorSetLayouts)
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

void PostPassthroughMaterial::createPipeline(vk::RenderPass renderPass)
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
