#pragma once

#include "cmx_input_action.h"
#include <functional>
#include <unordered_map>

// lib
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// std
#include <string>

namespace cmx
{

class CmxInputManager
{
  public:
    CmxInputManager(class CmxWindow &, const std::unordered_map<std::string, CmxInputAction *> &inputDictionary);
    ~CmxInputManager();

    void bindAxis(const std::string &, std::function<void(float, glm::vec2)>);
    void bindButton(const std::string &, std::function<void(float)>);
    void pollEvents(float dt);
    void setMouseCapture(bool);

  private:
    bool gamepadDetected{false};
    std::unordered_map<std::string, CmxInputAction *> inputDictionary;
    class CmxWindow &window;
};

} // namespace cmx
