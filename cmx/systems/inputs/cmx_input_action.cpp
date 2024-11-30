#include "cmx_input_action.h"

// cmx
#include "cmx/systems/rendering/cmx_window.h"

// lib
#include <GLFW/glfw3.h>
#include <glm/ext/scalar_constants.hpp>
#include <glm/geometric.hpp>
#include <spdlog/spdlog.h>

// std
#include <cstdlib>

namespace cmx
{

void ButtonAction::poll(const CmxWindow &window, float dt)
{
    int newStatus = 0;

    for (Button &button : buttons)
    {
        switch (button.source)
        {
        case InputSource::KEYBOARD:
            button.status = glfwGetKey(window.getGLFWwindow(), button.code);
            newStatus |= button.status;
            break;
        case InputSource::MOUSE:
            button.status = glfwGetMouseButton(window.getGLFWwindow(), button.code);
            newStatus |= button.status;
            break;
        case InputSource::GAMEPAD:
            // TODO: Gamepad support
            break;
        }
    }

    bool success = false;

    switch (buttonType)
    {
    case Type::HELD:
        if (newStatus == Type::HELD)
        {
            success = true;
        }
        break;

    case Type::PRESSED:
        if (newStatus == 1 && status == 0)
        {
            success = true;
        }
        break;

    case Type::RELEASED:
        if (newStatus == 0 && status == 1)
        {
            success = true;
        }
        break;
    }
    status = newStatus;

    if (success)
    {
        for (std::function<void(float)> func : functions)
        {
            func(dt);
        }
    }
}

void AxisAction::poll(const CmxWindow &window, float dt)
{
    switch (type)
    {
    case Type::BUTTONS:
        for (int i = 0; i < 4; i++)
        {
            if (buttons[i] == CMX_BUTTON_VOID)
            {
                continue;
            }

            switch (buttons[i].source)
            {
            case InputSource::KEYBOARD:
                buttons[i].status = glfwGetKey(window.getGLFWwindow(), buttons[i].code);
                break;
            case InputSource::MOUSE:
                buttons[i].status = glfwGetMouseButton(window.getGLFWwindow(), buttons[i].code);
                break;
            case InputSource::GAMEPAD:
                // TODO: Gamepad support
                break;
            }
        }

        value = glm::vec2{float(buttons[0].status - buttons[1].status), float(buttons[2].status - buttons[3].status)};

        break;
    case Type::AXES:
        for (int i = 0; i < 2; i++)
        {
            if (axes[i] == CMX_AXIS_VOID)
            {
                continue;
            }
            if (axes[i] == CMX_MOUSE_AXIS_X_ABSOLUTE)
            {
                double x, y;
                glfwGetCursorPos(window.getGLFWwindow(), &x, &y);
                axes[i].value = float(x);
            }
            if (axes[i] == CMX_MOUSE_AXIS_Y_ABSOLUTE)
            {
                double x, y;
                glfwGetCursorPos(window.getGLFWwindow(), &x, &y);
                axes[i].value = float(y);
            }
            if (axes[i] == CMX_MOUSE_AXIS_X_RELATIVE)
            {
                double x, y;
                glfwGetCursorPos(window.getGLFWwindow(), &x, &y);
                axes[i].value = float(x) - axes[i].absValue;
                axes[i].absValue = float(x);
            }
            if (axes[i] == CMX_MOUSE_AXIS_Y_RELATIVE)
            {
                double x, y;
                glfwGetCursorPos(window.getGLFWwindow(), &x, &y);
                axes[i].value = float(y) - axes[i].absValue;
                axes[i].absValue = float(y);
            }
        }

        value = glm::vec2{axes[0].value, axes[1].value};

        break;
    }

    if (glm::length(value) > glm::epsilon<float>())
    {
        for (std::function<void(float, glm::vec2)> func : functions)
        {
            func(dt, value);
        }
    }
}

void ButtonAction::bind(std::function<void(float)> callbackFunction)
{
    functions.push_back(callbackFunction);
}

void ButtonAction::bind(std::function<void(float, glm::vec2)> callbackFunction)
{
    spdlog::critical("ButtonAction: can only be bound to std::function<void(float)>");
    std::exit(EXIT_FAILURE);
}

void AxisAction::bind(std::function<void(float)> callbackFunction)
{
    spdlog::critical("AxisAction: can only be bound to std::function<void(float, glm::vec2)>");
    std::exit(EXIT_FAILURE);
}

void AxisAction::bind(std::function<void(float, glm::vec2)> callbackFunction)
{
    functions.push_back(callbackFunction);
}

} // namespace cmx
