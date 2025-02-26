#include "cmx_light_environment.h"

// cmx
#include ".external/imgui-gradient-hotfix/src/ColorRGBA.hpp"
#include ".external/imgui-gradient-hotfix/src/RelativePosition.hpp"
#include "cmx_render_system.h"

// lib
#include <imgui_gradient/imgui_gradient.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <spdlog/spdlog.h>

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
        ubo->pointLights[i].color = glm::vec4(*pair.second.lightColor, *pair.second.lightIntensity);
        i++;
    }
    ubo->numLights = i;

    calculateSun(ubo);
}

void LightEnvironment::calculateSun(GlobalUbo *ubo) const
{
    float theta = (_timeOfDay / 24.f) * glm::two_pi<float>() - glm::half_pi<float>();
    ubo->sun.direction = glm::vec4(glm::cos(theta), glm::sin(theta), 0.f, 1.f);

    float sunIntensity = 1.f - std::abs(_timeOfDay / 12.f - 1.f);
    ImGG::ColorRGBA sunColor = atmosphereWidget.gradient().at(ImGG::RelativePosition{sunIntensity});
    ubo->sun.color = glm::vec4(sunColor.x, sunColor.y, sunColor.z, sunIntensity);
    ubo->ambientLight = glm::vec4(sunColor.x, sunColor.y, sunColor.z, .05f);
}

void LightEnvironment::addPointLight(uint32_t id, PointLightStruct pointLight)
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

tinyxml2::XMLElement &LightEnvironment::save(tinyxml2::XMLDocument &, tinyxml2::XMLElement *element) const
{
    return *element;
}

void LightEnvironment::load(tinyxml2::XMLElement *)
{
}

void LightEnvironment::editor()
{
    ImGui::DragFloat("Time of day", &_timeOfDay, 0.25f, 0.0f, 23.99f, "%.2f");
    ImGui::DragFloat("Sun axis", &_sunAxis, 5.f, 0.f, 180.f, "%.0f");
    atmosphereWidget.widget("Atmosphere color");
}

} // namespace cmx
