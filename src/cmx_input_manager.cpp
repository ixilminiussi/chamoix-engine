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

void CmxInputManager::bind(const std::string &name, std::function<void(glm::vec2)> callbackFunction)
{
    auto mappedInput = inputDictionary.find(name);
    if (mappedInput == inputDictionary.end())
    {
        spdlog::warn("InputManager: attempt at binding non existant input '{0}'", name);
        return;
    }
    else
    {
        if (mappedInput->second.inputCategory != CmxInputCategory::AXIS)
        {
            spdlog::critical(
                "InputManager: failed to bind, AXIS inputs must be bound to functions of shape 'void (glm::vec2);");
            std::exit(EXIT_FAILURE);
        }
        mappedInput->second.bind(callbackFunction);
    }
}

void CmxInputManager::bind(const std::string &name, std::function<void(bool)> callbackFunction)
{
    auto mappedInput = inputDictionary.find(name);
    if (mappedInput == inputDictionary.end())
    {
        spdlog::warn("InputManager: attempt at binding non existant input '{0}'", name);
        return;
    }
    else
    {
        if (mappedInput->second.inputCategory != CmxInputCategory::BUTTON)
        {
            spdlog::critical(
                "InputManager: failed to bind, BUTTON inputs must be bound to functions of shape 'void (bool);");
            std::exit(EXIT_FAILURE);
        }
        mappedInput->second.bind(callbackFunction);
    }
}

void CmxInputManager::pollEvents()
{
    glfwPollEvents();

    for (auto actionPair : inputDictionary)
    {
        actionPair.second.poll(window);
    }
}

} // namespace cmx
