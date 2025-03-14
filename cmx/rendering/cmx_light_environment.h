#ifndef CMX_LIGHT_ENVIRONMENT
#define CMX_LIGHT_ENVIRONMENT

// lib
#include "tinyxml2.h"
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <vulkan/vulkan.hpp>

// std
#include <cstdint>
#include <unordered_map>

namespace cmx
{

struct PointLight
{
    PointLight() = default;
    PointLight(glm::vec3 *position_, glm::vec3 *color_, float *intensity_)
        : position(position_), color(color_), intensity(intensity_) {};
    glm::vec3 *position;
    glm::vec3 *color;
    float *intensity;

    void createImage();

  protected:
    vk::Image _image;
    vk::ImageView _imageView;
};

struct DirectionalLight
{
    DirectionalLight() = default;
    DirectionalLight(const glm::vec4 &direction_, const glm::vec4 &color_, const float &intensity_)
        : direction(direction_), color(color_), intensity(intensity_) {};

    glm::vec4 direction{1.f};
    glm::vec4 color{0.f};
    float intensity{0.f};

    void createImage();

  protected:
    vk::Image _image;
    vk::ImageView _imageView;
};

class LightEnvironment
{
  public:
    LightEnvironment();

    void addPointLight(uint32_t, const PointLight &);
    void removePointLight(uint32_t);

    void populateUbo(struct GlobalUbo *) const;

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &, tinyxml2::XMLElement *) const;
    void load(tinyxml2::XMLElement *);
    void loadDefaults();
    void editor();

  private:
    void calculateSun();

    std::unordered_map<uint32_t, PointLight> _pointLightsMap;
    DirectionalLight _sun;

    float _timeOfDay{11.f};
    float _sunAxis{0.f};

    bool _hasSun;

    glm::vec4 _ambientLighting{1.f, 1.f, 1.f, 0.2f};
};

} // namespace cmx

#endif
