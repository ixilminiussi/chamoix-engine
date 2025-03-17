#include "cmx_light_environment.h"

// cmx
#include "cmx_camera.h"
#include "cmx_drawable.h"
#include "cmx_frame_info.h"
#include "cmx_render_system.h"
#include "cmx_renderer.h"
#include "cmx_texture.h"
#include "imgui.h"

// lib
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <imgui_gradient/imgui_gradient.hpp>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace cmx
{

ImGG::GradientWidget atmosphereWidget;

void DirectionalLight::initializeShadowMap(class Device *device, uint32_t width, uint32_t height)
{
    vk::ImageCreateInfo imageInfo{};

    imageInfo.sType = vk::StructureType::eImageCreateInfo;
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.extent = vk::Extent3D{width, height, 1u};
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = vk::SampleCountFlagBits::e1;
    imageInfo.format = vk::Format::eD32Sfloat;
    imageInfo.tiling = vk::ImageTiling::eOptimal;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;

    device->createImageWithInfo(imageInfo, {vk::MemoryPropertyFlagBits::eDeviceLocal}, _image, _imageMemory);

    vk::ImageViewCreateInfo viewInfo{};
    viewInfo.image = _image; // The depth texture image
    viewInfo.viewType = vk::ImageViewType::e2D;
    viewInfo.format = vk::Format::eD32Sfloat;
    viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (device->device().createImageView(&viewInfo, nullptr, &_imageView) != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to create texture image view!");
    }

    vk::AttachmentDescription depthAttachment{};
    depthAttachment.format = vk::Format::eD32Sfloat;
    depthAttachment.samples = vk::SampleCountFlagBits::e1;
    depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    depthAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
    depthAttachment.finalLayout = vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimal;

    vk::AttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 0;
    depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::SubpassDescription subpass{};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    vk::RenderPassCreateInfo renderPassInfo{};
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &depthAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if (device->device().createRenderPass(&renderPassInfo, nullptr, &_renderPass) != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to create depth render pass");
    }

    vk::FramebufferCreateInfo framebufferInfo{};
    framebufferInfo.renderPass = _renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = &_imageView;
    framebufferInfo.width = width;
    framebufferInfo.height = height;
    framebufferInfo.layers = 1;

    if (device->device().createFramebuffer(&framebufferInfo, nullptr, &_framebuffer) != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to create frame buffer");
    }
}

void DirectionalLight::transitionShadowMap(class FrameInfo *frameInfo) const
{
    vk::ImageMemoryBarrier barrier{};
    barrier.image = _image;
    barrier.oldLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    barrier.newLayout = vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimal;
    barrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    frameInfo->commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eLateFragmentTests,
                                             vk::PipelineStageFlagBits::eFragmentShader, {}, 0, nullptr, 0, nullptr, 1,
                                             &barrier);
}

void DirectionalLight::freeShadowMap(class Device *device)
{
    device->device().destroyFramebuffer(_framebuffer);
    device->device().destroyRenderPass(_renderPass);
    device->device().destroyImageView(_imageView);
    device->device().destroyImage(_image);
    device->device().freeMemory(_imageMemory);
}

void DirectionalLight::beginRender(FrameInfo *frameInfo) const
{
    Camera *lightCamera = new Camera();
    glm::vec3 position = glm::vec3{0.f} - glm::vec3{direction} * 100.f;
    glm::vec3 upVector = glm::vec3(0.f, -1.f, 0.f);

    const float boundingDimension = 500.f;
    lightCamera->setOrthographicProjection(-boundingDimension, boundingDimension, boundingDimension, -boundingDimension,
                                           0, 100);
    lightCamera->setViewDirection(position, direction, glm::vec3{0.f, -1.f, 0.f});

    vk::ClearValue clearDepth{};
    clearDepth.depthStencil = 1.0f;

    vk::RenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.renderPass = _renderPass;
    renderPassBeginInfo.framebuffer = _framebuffer;
    renderPassBeginInfo.renderArea.extent.width = 1024;
    renderPassBeginInfo.renderArea.extent.height = 1024;
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearDepth;

    frameInfo->commandBuffer.beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);

    // GlobalUbo ubo{};
    // ubo.projection = frameInfo->camera->getProjection();
    // ubo.view = frameInfo->camera->getView();

    // RenderSystem::getInstance()->writeUboBuffers(&ubo);
}

void DirectionalLight::endRender(class FrameInfo *frameInfo) const
{
    frameInfo->commandBuffer.endRenderPass();
    transitionShadowMap(frameInfo);
}

LightEnvironment::LightEnvironment()
{
    _pointLightsMap.reserve(MAX_POINT_LIGHTS);

    _sun.initializeShadowMap(RenderSystem::getInstance()->getDevice(), 1024u, 1024u);
}

LightEnvironment::~LightEnvironment()
{
    _sun.freeShadowMap(RenderSystem::getInstance()->getDevice());
}

void LightEnvironment::drawShadowMaps(
    class FrameInfo *frameInfo,
    const std::map<uint8_t, std::vector<std::pair<class Drawable *, class DrawOption *>>> &drawableRenderQueue) const
{
    _sun.beginRender(frameInfo);

    for (auto &[materialID, drawableQueue] : drawableRenderQueue)
    {
        Texture::resetBoundID();
        for (auto &[drawable, drawOption] : drawableQueue)
        {
            drawable->render(*frameInfo, drawOption);
        }
    }

    _sun.endRender(frameInfo);
}

void LightEnvironment::populateUbo(GlobalUbo *ubo) const
{
    PointLight pointLights[MAX_POINT_LIGHTS];

    int i = 0;
    for (auto &pair : _pointLightsMap)
    {
        ubo->pointLights[i].position = glm::vec4(*pair.second.position, 1.0f);
        ubo->pointLights[i].color = glm::vec4(*pair.second.color, *pair.second.intensity);
        i++;
    }
    ubo->numLights = i;

    if (_hasSun)
    {
        ubo->sun.direction = _sun.direction;
        ubo->sun.color = _sun.color;
        ubo->ambientLight = glm::vec4(_sun.color.x, _sun.color.y, _sun.color.z, _sun.intensity * .1f + .05f);
    }
    else
    {
        ubo->sun.color = {1.f, 1.f, 1.f, 0.f};
        ubo->ambientLight = _ambientLighting;
    }
}

void LightEnvironment::calculateSun()
{
    float theta = (_timeOfDay / 24.f) * glm::two_pi<float>() - glm::half_pi<float>();
    _sun.direction = glm::vec4(glm::cos(theta), std::max(0.f, glm::sin(theta)), 0.f, 1.f);

    float sunIntensity = 1.f - std::abs(_timeOfDay / 12.f - 1.f);
    sunIntensity *= sunIntensity;
    ImGG::ColorRGBA sunColor = atmosphereWidget.gradient().at(ImGG::RelativePosition{sunIntensity});
    sunIntensity *= 2.f;

    _sun.color = glm::vec4(sunColor.x, sunColor.y, sunColor.z, sunIntensity);
    _sun.intensity = sunIntensity;
}

void LightEnvironment::addPointLight(uint32_t id, const PointLight &pointLight)
{
    if (_pointLightsMap.size() < MAX_POINT_LIGHTS)
    {
        _pointLightsMap[id] = pointLight;
    }
    else
    {
        spdlog::error("LightEnvironment: Reached maximum amount of point lights alloded by RenderSystem");
    }
}

void LightEnvironment::removePointLight(uint32_t id)
{
    _pointLightsMap.erase(id);
}

tinyxml2::XMLElement &LightEnvironment::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const
{
    tinyxml2::XMLElement *lightEnvironmentElement = doc.NewElement("lightEnvironment");
    lightEnvironmentElement->SetAttribute("timeOfDay", _timeOfDay);

    if (_hasSun)
    {
        tinyxml2::XMLElement *sunElement = doc.NewElement("sun");
        sunElement->SetAttribute("axis", _sunAxis);

        tinyxml2::XMLElement *gradientElement = doc.NewElement("gradient");

        for (const auto &mark : atmosphereWidget.gradient().get_marks())
        {
            tinyxml2::XMLElement *markElement = doc.NewElement("mark");
            markElement->SetAttribute("position", mark.position.get());
            markElement->SetAttribute("r", mark.color.x);
            markElement->SetAttribute("g", mark.color.y);
            markElement->SetAttribute("b", mark.color.z);
            gradientElement->InsertEndChild(markElement);
        }

        lightEnvironmentElement->InsertEndChild(sunElement);
        sunElement->InsertEndChild(gradientElement);
    }
    else
    {
        tinyxml2::XMLElement *ambientLightElement = doc.NewElement("ambientLight");
        ambientLightElement->SetAttribute("r", _ambientLighting.r);
        ambientLightElement->SetAttribute("g", _ambientLighting.g);
        ambientLightElement->SetAttribute("b", _ambientLighting.b);
        ambientLightElement->SetAttribute("a", _ambientLighting.a);

        lightEnvironmentElement->InsertEndChild(ambientLightElement);
    }

    parentElement->InsertEndChild(lightEnvironmentElement);

    return *lightEnvironmentElement;
}

void LightEnvironment::load(tinyxml2::XMLElement *parentElement)
{
    loadDefaults();

    if (tinyxml2::XMLElement *lightEnvironmentElement = parentElement->FirstChildElement("lightEnvironment"))
    {
        _timeOfDay = lightEnvironmentElement->FloatAttribute("timeOfDay");

        if (tinyxml2::XMLElement *sunElement = lightEnvironmentElement->FirstChildElement("sun"))
        {
            _hasSun = true;
            _sunAxis = sunElement->FloatAttribute("axis");

            if (tinyxml2::XMLElement *gradientELement = sunElement->FirstChildElement("gradient"))
            {
                tinyxml2::XMLElement *markElement = gradientELement->FirstChildElement("mark");

                atmosphereWidget.gradient().clear();
                while (markElement)
                {
                    ImGG::Mark mark{};
                    mark.position.set(markElement->FloatAttribute("position"));
                    mark.color.x = markElement->FloatAttribute("r");
                    mark.color.y = markElement->FloatAttribute("g");
                    mark.color.z = markElement->FloatAttribute("b");
                    atmosphereWidget.gradient().add_mark(mark);

                    markElement = markElement->NextSiblingElement("mark");
                }
            }

            calculateSun();
        }
        else
        {
            _hasSun = false;

            if (tinyxml2::XMLElement *ambientLightElement = lightEnvironmentElement->FirstChildElement("ambientLight"))
            {
                _ambientLighting.r = ambientLightElement->FloatAttribute("r");
                _ambientLighting.g = ambientLightElement->FloatAttribute("g");
                _ambientLighting.b = ambientLightElement->FloatAttribute("b");
                _ambientLighting.a = ambientLightElement->FloatAttribute("a");
            }
        }
    }
}

void LightEnvironment::loadDefaults()
{
    _timeOfDay = 10.f;

    atmosphereWidget.gradient().clear();

    ImGG::Mark mark_1{};
    mark_1.position.set(.352f);
    mark_1.color = {.55f, .50f, 1.f, 1.f};
    ImGG::Mark mark_2{};
    mark_2.position.set(.53f);
    mark_2.color = {1.f, .55f, 35.f, 1.f};
    ImGG::Mark mark_3{};
    mark_3.position.set(.61f);
    mark_3.color = {1.f, .86f, .53f, 1.f};
    ImGG::Mark mark_4{};
    mark_4.position.set(.78f);
    mark_4.color = {.78f, .85f, .95f, 1.f};
    atmosphereWidget.gradient().add_mark(mark_1);
    atmosphereWidget.gradient().add_mark(mark_2);
    atmosphereWidget.gradient().add_mark(mark_3);
    atmosphereWidget.gradient().add_mark(mark_4);

    _hasSun = false;
}

void LightEnvironment::editor()
{
    ImGui::Checkbox("has sun", &_hasSun);

    if (_hasSun)
    {
        ImGui::DragFloat("Time of day", &_timeOfDay, 0.25f, 0.0f, 23.99f, "%.2f");
        ImGui::DragFloat("Sun axis", &_sunAxis, 5.f, 0.f, 180.f, "%.0f");
        atmosphereWidget.widget("Atmosphere color");

        calculateSun();
    }
    else
    {
        ImGui::ColorPicker4("Ambient light", (float *)&_ambientLighting,
                            ImGuiColorEditFlags_Float && ImGuiColorEditFlags_InputRGB);
    }
}

} // namespace cmx
