#include "cmx_input_action.h"

// cmx
#include "cmx/systems/rendering/cmx_window.h"
#include "imgui.h"

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
            if (!ImGui::GetIO().WantCaptureKeyboard)
            {
                button.status = glfwGetKey(window.getGLFWwindow(), button.code);

                if (buttonType == Type::SHORTCUT)
                {
                    newStatus &= button.status;
                }
                else
                {
                    newStatus |= button.status;
                }
            }
            break;
        case InputSource::MOUSE:
            if (!ImGui::GetIO().WantCaptureMouse)
            {
                button.status = glfwGetMouseButton(window.getGLFWwindow(), button.code);
                if (buttonType == Type::SHORTCUT)
                {
                    newStatus &= button.status;
                }
                else
                {
                    newStatus |= button.status;
                }
            }
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
        if (newStatus == 1)
        {
            success = true;
        }
        break;

    case Type::PRESSED:
    case Type::SHORTCUT:
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
    case Type::TOGGLE:
        if (newStatus != status)
        {
            success = true;
        }
        break;
    }
    status = newStatus;

    if (success)
    {
        for (std::function<void(float, int)> func : functions)
        {
            func(dt, newStatus);
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
                if (!ImGui::GetIO().WantCaptureKeyboard)
                {
                    buttons[i].status = glfwGetKey(window.getGLFWwindow(), buttons[i].code);
                }
                break;
            case InputSource::MOUSE:
                if (!ImGui::GetIO().WantCaptureMouse)
                {
                    buttons[i].status = glfwGetMouseButton(window.getGLFWwindow(), buttons[i].code);
                }
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

void ButtonAction::bind(std::function<void(float, int)> callbackFunction)
{
    functions.push_back(callbackFunction);
}

void ButtonAction::bind(std::function<void(float, glm::vec2)> callbackFunction)
{
    spdlog::critical("ButtonAction: can only be bound to std::function<void(float, int)>");
    std::exit(EXIT_FAILURE);
}

tinyxml2::XMLElement &ButtonAction::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement)
{
    tinyxml2::XMLElement *inputActionElement = doc.NewElement("buttonAction");

    switch (buttonType)
    {
    case (Type::PRESSED):
        inputActionElement->SetAttribute("type", "PRESSED");
        break;
    case (Type::HELD):
        inputActionElement->SetAttribute("type", "HELD");
        break;
    case (Type::RELEASED):
        inputActionElement->SetAttribute("type", "RELEASED");
        break;
    case (Type::SHORTCUT):
        inputActionElement->SetAttribute("type", "SHORTCUT");
        break;
    case (Type::TOGGLE):
        inputActionElement->SetAttribute("type", "TOGGLE");
        break;
    }

    for (Button button : buttons)
    {
        tinyxml2::XMLElement *buttonElement = doc.NewElement("button");
        buttonElement->SetAttribute("code", button.code);

        switch (button.source)
        {
        case (InputSource::GAMEPAD):
            buttonElement->SetAttribute("source", "GAMEPAD");
            break;
        case (InputSource::MOUSE):
            buttonElement->SetAttribute("source", "MOUSE");
            break;
        case (InputSource::KEYBOARD):
            buttonElement->SetAttribute("source", "KEYBOARD");
            break;
        }

        inputActionElement->InsertEndChild(buttonElement);
    }

    parentElement->InsertEndChild(inputActionElement);
    return *inputActionElement;
}

void ButtonAction::load(tinyxml2::XMLElement *parentElement)
{
}

void AxisAction::bind(std::function<void(float, int)> callbackFunction)
{
    spdlog::critical("AxisAction: can only be bound to std::function<void(float, glm::vec2)>");
    std::exit(EXIT_FAILURE);
}

void AxisAction::bind(std::function<void(float, glm::vec2)> callbackFunction)
{
    functions.push_back(callbackFunction);
}

tinyxml2::XMLElement &AxisAction::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement)
{
    tinyxml2::XMLElement *inputActionElement = doc.NewElement("axisAction");

    switch (type)
    {
    case (Type::BUTTONS):
        inputActionElement->SetAttribute("type", "BUTTONS");

        for (Axis axis : axes)
        {
            tinyxml2::XMLElement *axisElement = doc.NewElement("axis");
            axisElement->SetAttribute("code", axis.code);

            switch (axis.source)
            {
            case (InputSource::GAMEPAD):
                axisElement->SetAttribute("source", "GAMEPAD");
                break;
            case (InputSource::MOUSE):
                axisElement->SetAttribute("source", "MOUSE");
                break;
            case (InputSource::KEYBOARD):
                axisElement->SetAttribute("source", "KEYBOARD");
                break;
            }

            inputActionElement->InsertEndChild(axisElement);
        }

        break;
    case (Type::AXES):
        inputActionElement->SetAttribute("type", "AXES");

        for (Button button : buttons)
        {
            tinyxml2::XMLElement *axisElement = doc.NewElement("axis");
            axisElement->SetAttribute("code", button.code);

            switch (button.source)
            {
            case (InputSource::GAMEPAD):
                axisElement->SetAttribute("source", "GAMEPAD");
                break;
            case (InputSource::MOUSE):
                axisElement->SetAttribute("source", "MOUSE");
                break;
            case (InputSource::KEYBOARD):
                axisElement->SetAttribute("source", "KEYBOARD");
                break;
            }

            inputActionElement->InsertEndChild(axisElement);
        }

        break;
    }

    parentElement->InsertEndChild(inputActionElement);
    return *inputActionElement;
}

void AxisAction::load(tinyxml2::XMLElement *parentElement)
{
}

} // namespace cmx
