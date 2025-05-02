#include "cmx_parallax_material.h"

// cmx
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

void ParallaxMaterial::bind(FrameInfo *frameInfo, const Drawable *drawable)
{
    if (_boundID != _id)
    {
        _pipeline->bind(frameInfo->commandBuffer);

        frameInfo->commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipelineLayout, 0, 1,
                                                    &frameInfo->globalDescriptorSet, 0, nullptr);

        const std::vector<size_t> &descriptorSetIDs = GraphicsManager::getDescriptorSetIDs();
        frameInfo->commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipelineLayout, 3, 1,
                                                    &(_renderSystem->getSamplerDescriptorSet(descriptorSetIDs[0])), 0,
                                                    nullptr);

        _boundID = _id;
    }

    PushConstantData push{};
    Transform transform = drawable->getWorldSpaceTransform();

    push.modelMatrix = transform.mat4();
    push.normalMatrix = transform.normalMatrix();

    push.normalMatrix[0][3] = _UVoffset.x;
    push.normalMatrix[1][3] = _UVoffset.y;
    push.normalMatrix[2][3] = _worldSpaceUV ? _UVScale : 0.f;
    push.normalMatrix[3][3] = glm::radians(_UVRotate);
    push.normalMatrix[3][2] = _parallaxDepth;
    push.normalMatrix[3][1] = _occlusionMapping ? static_cast<float>(_parallaxLevels) : 1.f;

    frameInfo->commandBuffer.pushConstants(_pipelineLayout,
                                           vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0,
                                           sizeof(PushConstantData), &push);
}

void ParallaxMaterial::editor()
{
    Material::editor();

    ImGui::Checkbox("World space UV", &_worldSpaceUV);
    if (_worldSpaceUV)
    {
        ImGui::SliderFloat2("UV offset", (float *)&_UVoffset, -1.f, 1.f);
        ImGui::DragFloat("Scale", &_UVScale);
        ImGui::DragFloat("Rotate", &_UVRotate, 1.f, -180.f, 180.f);
    }

    ImGui::SeparatorText("Parallax");
    ImGui::DragFloat("Depth", &_parallaxDepth, .01f, 0.f, .1f);
    ImGui::Checkbox("Occlusion Mapping", &_occlusionMapping);
    if (_occlusionMapping)
    {
        ImGui::DragInt("Layer Resolution", &_parallaxLevels, 1, 1, 64);
    }
}

tinyxml2::XMLElement *ParallaxMaterial::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const
{
    tinyxml2::XMLElement *materialElement = Material::save(doc, parentElement);
    if (materialElement == nullptr)
        return nullptr;

    materialElement->SetAttribute("worldSpaceUV", _worldSpaceUV);
    if (_worldSpaceUV)
    {
        materialElement->SetAttribute("UVoffsetX", _UVoffset.x);
        materialElement->SetAttribute("UVoffsetY", _UVoffset.y);
        materialElement->SetAttribute("UVscale", _UVScale);
        materialElement->SetAttribute("UVrotate", _UVRotate);
    }

    materialElement->SetAttribute("parallaxDepth", _parallaxDepth);
    materialElement->SetAttribute("occlusionMapping", _occlusionMapping);
    if (_occlusionMapping)
    {
        materialElement->SetAttribute("layerCount", _parallaxLevels);
    }

    return materialElement;
}

void ParallaxMaterial::load(tinyxml2::XMLElement *materialElement)
{
    Material::load(materialElement);

    _worldSpaceUV = materialElement->BoolAttribute("worldSpaceUV");
    if (_worldSpaceUV)
    {
        _UVoffset =
            glm::vec2{materialElement->FloatAttribute("UVoffsetX"), materialElement->FloatAttribute("UVoffsetY")};
        _UVScale = materialElement->FloatAttribute("UVscale");
        _UVRotate = materialElement->FloatAttribute("UVrotate");
    }

    _parallaxDepth = materialElement->FloatAttribute("parallaxDepth");
    _occlusionMapping = materialElement->BoolAttribute("occlusionMapping");
    if (_occlusionMapping)
    {
        _parallaxLevels = materialElement->IntAttribute("layerCount");
    }
}

void ParallaxMaterial::initialize()
{
    RenderSystem *renderSystem = RenderSystem::getInstance();

    loadBindings();

    createPipelineLayout({renderSystem->getGlobalSetLayout(), renderSystem->getSamplerDescriptorSetLayout(),
                          renderSystem->getSamplerDescriptorSetLayout(),
                          renderSystem->getSamplerDescriptorSetLayout()});
    createPipeline(renderSystem->getRenderPass());
}

void ParallaxMaterial::createPipelineLayout(std::vector<vk::DescriptorSetLayout> descriptorSetLayouts)
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

void ParallaxMaterial::createPipeline(vk::RenderPass renderPass)
{
    assert(_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = _pipelineLayout;
    _pipeline = std::make_unique<Pipeline>(*_renderSystem->getDevice(), _vertFilepath, _fragFilepath, pipelineConfig,
                                           "parallax material pipeline");
}

} // namespace cmx
