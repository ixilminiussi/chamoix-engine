#include "cmx_input_action.h"
#include "cmx_window.h"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <initializer_list>
#include <spdlog/spdlog.h>

namespace cmx
{

void CmxInputAction::validateInputs()
{
    switch (inputCategory)
    {
    case CmxInputCategory::BUTTON:
        for (const CmxInput &input : inputs)
        {
            if (input.type != CmxInputType::KEYBOARD && input.type != CmxInputType::GAMEPAD &&
                input.type != CmxInputType::MOUSE)
            {
                spdlog::critical("InputAction: Input of type '{0}' and BUTTON inputs are incompatible",
                                 (int)input.type);
                std::exit(EXIT_FAILURE);
            }
        }
        break;
    case cmx::CmxInputCategory::AXIS:
        for (CmxInput input : inputs)
        {
            if (input.type != CmxInputType::GAMEPAD_AXIS && input.type != CmxInputType::MOUSE_AXIS)
            {
                spdlog::critical("InputAction: Input of type '{0}' and AXIS inputs are incompatible", (int)input.type);
                std::exit(EXIT_FAILURE);
            }
        }
        break;
    }
}

CmxInputAction::CmxInputAction(const CmxInputCategory inputCategory, std::initializer_list<CmxInput> inputs)
    : inputCategory{inputCategory}, inputs{inputs}
{
    validateInputs();
}

void CmxInputAction::bind(std::function<void(bool)> callbackFunction)
{
    if (inputCategory != CmxInputCategory::BUTTON)
    {
        spdlog::critical("InputAction: function shape void(*)(bool) can only be bound to BUTTON inputs");
        std::exit(EXIT_FAILURE);
    }

    buttonFunctions.push_back(callbackFunction);
}

void CmxInputAction::bind(std::function<void(glm::vec2)> callbackFunction)
{
    if (inputCategory != CmxInputCategory::AXIS)
    {
        spdlog::critical("InputAction: function shape void(*)(glm::vec2) can only be bound to AXIS inputs");
        std::exit(EXIT_FAILURE);
    }

    axisFunctions.push_back(callbackFunction);
}

void CmxInputAction::poll(const CmxWindow &window)
{
    // TODO:
    // Instead of this, make each cmxInput hold their value themselves, update the various states on a per cmxInput
    // basis and then figure out which callback to call
    switch (inputCategory)
    {
    case CmxInputCategory::BUTTON:
        for (CmxInput &input : inputs)
        {
            if (input.type == CmxInputType::KEYBOARD)
            {
                int state = glfwGetKey(window.getGLFWwindow(), input.code);

                if (state != input.status)
                {
                    input.status = state; // TODO: investigate why this doesn't do shit

                    for (std::function<void(bool)> callbackFunction : buttonFunctions)
                    {
                        callbackFunction(state);
                    }
                }
            }
            // TODO: GAMEPAD SUPPORT
        }
        break;

    case CmxInputCategory::AXIS:
        break;
    }
}

} // namespace cmx
