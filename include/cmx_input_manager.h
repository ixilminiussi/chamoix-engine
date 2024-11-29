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
    CmxInputManager(class CmxWindow &);
    ~CmxInputManager() = default;

    void bind(const std::string &, std::function<void(glm::vec2)>);
    void bind(const std::string &, std::function<void(bool)>);
    void pollEvents();

  private:
    bool gamepadDetected{false};
    std::unordered_map<std::string, class CmxInputAction> inputDictionary = {
        {"forward_move", {CmxInputCategory::BUTTON, {CMX_KEY_W, CMX_KEY_UP}}},
    };
    class CmxWindow &window;
};

} // namespace cmx
