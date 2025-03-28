#ifndef CMX_LIGHT_ENVIRONMENT
#define CMX_LIGHT_ENVIRONMENT

// lib
#include "tinyxml2.h"
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/fwd.hpp>
#include <map>
#include <vulkan/vulkan.hpp>

// std
#include <cstdint>
#include <unordered_map>
#include <vulkan/vulkan_handles.hpp>

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
};

class Material;

class DirectionalLight
{
  public:
    DirectionalLight() = default;
    DirectionalLight(const glm::vec4 &direction_, const glm::vec4 &color_, const float &intensity_)
        : direction(direction_), color(color_), intensity(intensity_) {};

    glm::vec4 direction{1.f};
    glm::vec4 color{0.f};
    float intensity{0.f};

    friend class LightEnvironment;

  private:
    void initializeShadowMap(class Device *, uint32_t width, uint32_t height);
    void freeShadowMap(class Device *);
    [[nodiscard]] Material *beginRender(class FrameInfo *) const;
    [[nodiscard]] size_t endRender(class FrameInfo *) const;
    void transitionShadowMap(class FrameInfo *) const;

    vk::Image _image;
    vk::ImageView _imageView;
    vk::Sampler _sampler;
    size_t _samplerDescriptorSetID;
    vk::DeviceMemory _imageMemory;
    vk::RenderPass _renderPass;
    vk::Framebuffer _framebuffer;

    class VoidMaterial *_voidMaterial;

    vk::Extent2D _imageResolution{1024, 1024};
};

class LightEnvironment
{
  public:
    LightEnvironment();
    ~LightEnvironment();

    void addPointLight(uint32_t, const PointLight &);
    void removePointLight(uint32_t);

    void populateUbo(struct GlobalUbo *) const;

    void drawShadowMaps(class FrameInfo *,
                        const std::map<uint8_t, std::vector<std::pair<class Drawable *, class DrawOption *>>> &,
                        std::vector<size_t> &descriptorSetIDs) const;
    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &, tinyxml2::XMLElement *) const;
    void load(tinyxml2::XMLElement *);
    void unload();
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
