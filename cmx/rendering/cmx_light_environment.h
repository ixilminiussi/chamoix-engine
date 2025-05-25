#ifndef CMX_LIGHT_ENVIRONMENT
#define CMX_LIGHT_ENVIRONMENT

// lib
#include "cmx_descriptors.h"
#include "tinyxml2.h"
#include <glm/ext/matrix_common.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/fwd.hpp>
#include <memory>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

// std
#include <cstdint>
#include <map>
#include <unordered_map>

namespace cmx
{

struct ShadowUbo
{
    glm::mat4 projection;
    glm::mat4 view;
};

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
    DirectionalLight();
    DirectionalLight(const glm::vec4 &direction_, const glm::vec4 &color_, const float &intensity_);

    glm::vec4 direction{1.f};
    glm::vec4 color{0.f};
    float intensity{0.f};

    friend class LightEnvironment;
    const class Camera *getCameraView() const;

  private:
    void initializeShadowMap(class Device *, uint32_t width, uint32_t height);
    void createImage(class Device *);
    void createImageView(class Device *);
    void createRenderPass(class Device *);
    void createFrameBuffer(class Device *);
    void createSampler(class Device *);
    void createShadowUbo(class Device *);
    void createDescriptorSet(class Device *);

    void freeShadowMap(class Device *);
    [[nodiscard]] Material *beginRender(struct FrameInfo *) const;
    [[nodiscard]] size_t endRender(struct FrameInfo *) const;

    vk::Image _image;
    vk::ImageView _imageView;
    vk::Sampler _sampler;
    size_t _samplerDescriptorSetID;
    vk::DeviceMemory _imageMemory;
    vk::RenderPass _renderPass;
    vk::Framebuffer _framebuffer;

    float _boundingDimension = 10.f;

    class VoidMaterial *_voidMaterial;
    std::unique_ptr<class DescriptorPool> _shadowDescriptorPool;
    std::unique_ptr<class DescriptorSetLayout> _shadowDescriptorSetLayout;
    vk::DescriptorSet _shadowDescriptorSet;
    class Buffer *_shadowUboBuffer;

    std::unique_ptr<class Camera> _cameraView;

    vk::Extent2D _imageResolution;
};

class LightEnvironment
{
  public:
    LightEnvironment();
    ~LightEnvironment();

    void addPointLight(uint32_t, const PointLight &);
    void removePointLight(uint32_t);

    void populateUbo(struct GlobalUbo *) const;

    glm::vec4 getAmbientLighting() const;
    void drawShadowMaps(struct FrameInfo *,
                        const std::map<uint8_t, std::vector<std::pair<class Drawable *, struct DrawOption *>>> &,
                        std::vector<size_t> &descriptorSetIDs);
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

    bool _updateShadowMap{true};
};

} // namespace cmx

#endif
