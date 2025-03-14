#include "cmx_light_environment.h"

// cmx
#include "cmx_render_system.h"
#include "imgui.h"

// lib
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <imgui_gradient/imgui_gradient.hpp>
#include <spdlog/spdlog.h>
#include <vk_video/vulkan_video_codec_av1std.h>

namespace cmx
{

ImGG::GradientWidget atmosphereWidget;

LightEnvironment::LightEnvironment()
{
    _pointLightsMap.reserve(MAX_POINT_LIGHTS);
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
    mark_1.position.set(0.f);
    mark_1.color = {.55f, .50f, 1.f, 1.f};
    ImGG::Mark mark_2{};
    mark_2.position.set(0.4f);
    mark_2.color = {1.f, .48f, 40.f, 1.f};
    ImGG::Mark mark_3{};
    mark_3.position.set(0.58f);
    mark_3.color = {1.f, .60f, .24f, 1.f};
    ImGG::Mark mark_4{};
    mark_4.position.set(1.f);
    mark_4.color = {.9f, .97f, 1.f, 1.f};
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
