#ifndef CMX_GRAPHICS_MANAGER
#define CMX_GRAPHICS_MANAGER

// lib
#include <glm/ext/vector_float3.hpp>

// std
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

namespace cmx
{

struct PointLightStruct
{
    glm::vec3 *position;
    glm::vec3 *lightColor;
    float *lightIntensity;
};

class GraphicsManager
{
  public:
    GraphicsManager(std::map<uint8_t, std::shared_ptr<class RenderSystem>> &);

    void addToQueue(std::shared_ptr<class Component>);
    void removeFromQueue(std::shared_ptr<class Component>);

    void addPointLight(uint32_t, PointLightStruct);
    void removePointLight(uint32_t);

    void drawComponents(std::weak_ptr<class Camera>);

    void editor();

  private:
    std::map<uint8_t, std::shared_ptr<class RenderSystem>> &_renderSystems;
    std::map<uint8_t, std::vector<std::shared_ptr<class Component>>> _componentRenderQueue{};
    std::unordered_map<uint32_t, PointLightStruct> _pointLightsMap;

    // warning flags
    bool _noCameraFlag{false};
};

} // namespace cmx

#endif
