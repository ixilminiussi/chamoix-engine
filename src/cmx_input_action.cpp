#include "cmx_input_action.h"
#include "cmx_window.h"

// lib
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

// std
#include <cstdlib>

namespace cmx
{

void CmxButtonAction::poll(const CmxWindow &window, float dt)
{
    int newStatus = 0;

    for (CmxButton &button : buttons)
    {
        switch (button.source)
        {
        case CmxInputSource::KEYBOARD:
            button.status = glfwGetKey(window.getGLFWwindow(), button.code);
            newStatus |= button.status;
            break;
        case CmxInputSource::MOUSE:
            button.status = glfwGetMouseButton(window.getGLFWwindow(), button.code);
            newStatus |= button.status;
            break;
        case CmxInputSource::GAMEPAD:
            // TODO: Gamepad support
            break;
        }
    }

    bool success{false};

    switch (buttonType)
    {
    case Type::HELD:
        if (newStatus == Type::HELD)
        {
            success = true;
        }
        break;

    case Type::PRESSED:
        if (newStatus == Type::PRESSED && status == Type::RELEASED)
        {
            status = newStatus;
            success = true;
        }

        break;

    case Type::RELEASED:
        if (newStatus == Type::RELEASED && status == Type::PRESSED)
        {
            status = newStatus;
            success = true;
        }
        break;
    }

    if (success)
    {
        for (std::function<void(float)> func : functions)
        {
            func(dt);
        }
    }
}

void CmxAxisAction::poll(const CmxWindow &window, float dt)
{
    switch (type)
    {
    case Type::BUTTONS:
        for (int i = 0; i < 4; i++)
        {
            if (buttons[i].code == -1)
            {
                continue;
            }

            switch (buttons[i].source)
            {
            case CmxInputSource::KEYBOARD:
                buttons[i].status = glfwGetKey(window.getGLFWwindow(), buttons[i].code);
                break;
            case CmxInputSource::MOUSE:
                buttons[i].status = glfwGetMouseButton(window.getGLFWwindow(), buttons[i].code);
                break;
            case CmxInputSource::GAMEPAD:
                // TODO: Gamepad support
                break;
            }
        }

        value = glm::vec2{float(buttons[0].status - buttons[1].status), float(buttons[2].status - buttons[3].status)};

        break;
    case Type::AXES:
        break;
    }

    for (std::function<void(float, glm::vec2)> func : functions)
    {
        func(dt, value);
    }
}

void CmxButtonAction::bind(std::function<void(float)> callbackFunction)
{
    functions.push_back(callbackFunction);
}

void CmxButtonAction::bind(std::function<void(float, glm::vec2)> callbackFunction)
{
    spdlog::critical("ButtonAction: can only be bound to std::function<void(float)>");
    std::exit(EXIT_FAILURE);
}

void CmxAxisAction::bind(std::function<void(float)> callbackFunction)
{
    spdlog::critical("AxisAction: can only be bound to std::function<void(float, glm::vec2)>");
    std::exit(EXIT_FAILURE);
}

void CmxAxisAction::bind(std::function<void(float, glm::vec2)> callbackFunction)
{
    functions.push_back(callbackFunction);
}

} // namespace cmx
