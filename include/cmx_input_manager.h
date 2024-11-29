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
    ~CmxInputManager();

    void bindAxis(const std::string &, std::function<void(float, glm::vec2)>);
    void bindButton(const std::string &, std::function<void(float)>);
    void pollEvents(float dt);

  private:
    bool gamepadDetected{false};
    std::unordered_map<std::string, CmxInputAction *> inputDictionary = {
        {
            "jump",
            new CmxButtonAction{CmxButtonAction::Type::PRESSED, {CMX_KEY_W, CMX_KEY_UP}},
        },
        {"lateral movement", new CmxAxisAction{CMX_KEY_W, CMX_KEY_S, CMX_KEY_A, CMX_KEY_D}}};
    class CmxWindow &window;
};

} // namespace cmx
