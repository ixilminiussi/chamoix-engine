#include "cmx_input_manager.h"

#include "cmx_input_action.h"
#include "cmx_window.h"

// lib
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <spdlog/spdlog.h>

namespace cmx
{

CmxInputManager::CmxInputManager(CmxWindow &window) : window{window}
{
    gamepadDetected = glfwJoystickPresent(GLFW_JOYSTICK_1);
    glfwSetInputMode(window.getGLFWwindow(), GLFW_STICKY_KEYS, GLFW_TRUE);
}

CmxInputManager::~CmxInputManager()
{
    for (auto &pair : inputDictionary)
    {
        delete pair.second;
    }
    inputDictionary.clear();
}

void CmxInputManager::bindAxis(const std::string &name, std::function<void(float, glm::vec2)> callbackFunction)
{
    auto mappedInput = inputDictionary.find(name);
    if (mappedInput == inputDictionary.end())
    {
        spdlog::warn("InputManager: attempt at binding non existant input '{0}'", name);
        return;
    }
    else
    {
        mappedInput->second->bind(callbackFunction);
        spdlog::info("InputManager: '{0}' bound to new function", name);
    }
}

void CmxInputManager::bindButton(const std::string &name, std::function<void(float)> callbackFunction)
{
    auto mappedInput = inputDictionary.find(name);
    if (mappedInput == inputDictionary.end())
    {
        spdlog::warn("InputManager: attempt at binding non existant input '{0}'", name);
        return;
    }
    else
    {
        mappedInput->second->bind(callbackFunction);
        spdlog::info("InputManager: '{0}' bound to new function", name);
    }
}

void CmxInputManager::pollEvents(float dt)
{
    glfwPollEvents();

    for (auto &[name, input] : inputDictionary)
    {
        input->poll(window, dt);
    }
}

} // namespace cmx
