#ifndef CMX_LIGHT_ENVIRONMENT
#define CMX_LIGHT_ENVIRONMENT

// lib
#include "tinyxml2.h"
#include <glm/ext/vector_float3.hpp>

// std
#include <cstdint>
#include <unordered_map>

namespace cmx
{

struct PointLightStruct
{
    glm::vec3 *position;
    glm::vec3 *lightColor;
    float *lightIntensity;
};

class LightEnvironment
{
  public:
    LightEnvironment();

    void addPointLight(uint32_t, PointLightStruct);
    void removePointLight(uint32_t);

    void populateUbo(struct GlobalUbo *) const;

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &, tinyxml2::XMLElement *) const;
    void load(tinyxml2::XMLElement *);
    void loadDefaults();
    void editor();

  private:
    void calculateSun(struct GlobalUbo *) const;

    std::unordered_map<uint32_t, PointLightStruct> _pointLightsMap;

    float _timeOfDay{11.f};
    float _sunAxis{0.f};
};

} // namespace cmx

#endif
