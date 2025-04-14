#include "cmx_dithered_material.h"

// cmx
#include "cmx_camera.h"
#include "cmx_drawable.h"
#include "cmx_frame_info.h"
#include "cmx_graphics_manager.h"
#include "cmx_pipeline.h"
#include "cmx_render_system.h"
#include "cmx_renderer.h"

// lib
#include <imgui.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace cmx
{

void DitheredMaterial::bind(FrameInfo *frameInfo, const Drawable *drawable)
{
    if (_boundID != _id)
    {
        _pipeline->bind(frameInfo->commandBuffer);

        frameInfo->commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipelineLayout, 0, 1,
                                                    &frameInfo->globalDescriptorSet, 0, nullptr);

        const std::vector<size_t> &descriptorSetIDs = GraphicsManager::getDescriptorSetIDs();
        frameInfo->commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipelineLayout, 2, 1,
                                                    &(_renderSystem->getSamplerDescriptorSet(descriptorSetIDs[0])), 0,
                                                    nullptr);

        _boundID = _id;
    }

    PushConstantData push{};
    Transform transform = drawable->getWorldSpaceTransform();

    push.modelMatrix = transform.mat4();
    push.normalMatrix = transform.normalMatrix();

    push.normalMatrix[3][0] = _scale;
    push.normalMatrix[3][1] = _threshold;
    push.normalMatrix[3][2] = _lightDots;

    push.normalMatrix[0][3] = _lightColor.x;
    push.normalMatrix[1][3] = _lightColor.y;
    push.normalMatrix[2][3] = _lightColor.z;

    push.modelMatrix[0][3] = _darkColor.x;
    push.modelMatrix[1][3] = _darkColor.y;
    push.modelMatrix[2][3] = _darkColor.z;

    frameInfo->commandBuffer.pushConstants(_pipelineLayout,
                                           vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0,
                                           sizeof(PushConstantData), &push);
}

void DitheredMaterial::editor()
{
    Material::editor();

    ImGui::DragFloat("Scale", &_scale, 0.1f, 1.f, 8.f);
    ImGui::SliderFloat("Threshold", &_threshold, 0.5f, 2.f);
    ImGui::ColorEdit3("Light Color", (float *)&_lightColor);
    ImGui::ColorEdit3("Dark Color", (float *)&_darkColor);

    static const char *selected = _lightDots ? "light dots" : "dark dots";
    if (ImGui::BeginCombo("toggle", selected))
    {
        if (ImGui::Selectable("light dots", _lightDots))
        {
            selected = "light dots";
            _lightDots = true;
        }
        if (ImGui::Selectable("dark dots", !_lightDots))
        {
            selected = "dark dots";
            _lightDots = false;
        }
        ImGui::EndCombo();
    }
}

tinyxml2::XMLElement *DitheredMaterial::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const
{
    tinyxml2::XMLElement *materialElement = Material::save(doc, parentElement);
    if (materialElement == nullptr)
        return nullptr;

    materialElement->SetAttribute("scale", _scale);
    materialElement->SetAttribute("threshold", _threshold);
    materialElement->SetAttribute("dotsToggle", _lightDots);

    tinyxml2::XMLElement *lightColorElement = doc.NewElement("lightColor");
    lightColorElement->SetAttribute("r", _lightColor.r);
    lightColorElement->SetAttribute("g", _lightColor.g);
    lightColorElement->SetAttribute("b", _lightColor.b);
    materialElement->InsertEndChild(lightColorElement);

    tinyxml2::XMLElement *darkColorElement = doc.NewElement("darkColor");
    darkColorElement->SetAttribute("r", _darkColor.r);
    darkColorElement->SetAttribute("g", _darkColor.g);
    darkColorElement->SetAttribute("b", _darkColor.b);
    materialElement->InsertEndChild(darkColorElement);

    return materialElement;
}

void DitheredMaterial::load(tinyxml2::XMLElement *materialElement)
{
    Material::load(materialElement);

    _scale = materialElement->FloatAttribute("scale", 5.f);
    _threshold = materialElement->FloatAttribute("threshold", 1.f);
    _lightDots = materialElement->BoolAttribute("dotsToggle", true);

    if (tinyxml2::XMLElement *lightColorElement = materialElement->FirstChildElement("lightColor"))
    {
        _lightColor.r = lightColorElement->FloatAttribute("r", 1.f);
        _lightColor.g = lightColorElement->FloatAttribute("g", 1.f);
        _lightColor.b = lightColorElement->FloatAttribute("b", 1.f);
    }

    if (tinyxml2::XMLElement *darkColorElement = materialElement->FirstChildElement("darkColor"))
    {
        _darkColor.r = darkColorElement->FloatAttribute("r", 0.f);
        _darkColor.g = darkColorElement->FloatAttribute("g", 0.f);
        _darkColor.b = darkColorElement->FloatAttribute("b", 0.f);
    }
}

void DitheredMaterial::initialize()
{
    RenderSystem *renderSystem = RenderSystem::getInstance();

    loadBindings();

    createPipelineLayout({renderSystem->getGlobalSetLayout(), renderSystem->getSamplerDescriptorSetLayout(),
                          renderSystem->getSamplerDescriptorSetLayout()});
    createPipeline(renderSystem->getRenderPass());
}

void DitheredMaterial::createPipelineLayout(std::vector<vk::DescriptorSetLayout> descriptorSetLayouts)
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

void DitheredMaterial::createPipeline(vk::RenderPass renderPass)
{
    assert(_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = _pipelineLayout;
    _pipeline = std::make_unique<Pipeline>(*_renderSystem->getDevice(), _vertFilepath, _fragFilepath, pipelineConfig,
                                           "dithered material pipeline");
}

} // namespace cmx
