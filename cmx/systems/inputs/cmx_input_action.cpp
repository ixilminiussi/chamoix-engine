#include "cmx_input_action.h"

// cmx
#include "cmx_inputs.h"
#include "cmx_window.h"
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

const char *toString(ButtonAction::Type buttonType)
{
    switch (buttonType)
    {
    case ButtonAction::Type::HELD:
        return "HELD";
        break;
    case ButtonAction::Type::PRESSED:
        return "PRESSED";
        break;
    case ButtonAction::Type::RELEASED:
        return "RELEASED";
        break;
    case ButtonAction::Type::SHORTCUT:
        return "SHORTCUT";
        break;
    case ButtonAction::Type::TOGGLE:
        return "TOGGLE";
        break;
    }
    return "";
}

const ButtonAction::Type toButtonActionType(const char *typeString)
{
    if (strcmp(typeString, "PRESSED") == 0)
    {
        return ButtonAction::Type::PRESSED;
    }
    if (strcmp(typeString, "HELD") == 0)
    {
        return ButtonAction::Type::HELD;
    }
    if (strcmp(typeString, "RELEASED") == 0)
    {
        return ButtonAction::Type::RELEASED;
    }
    if (strcmp(typeString, "SHORTCUT") == 0)
    {
        return ButtonAction::Type::SHORTCUT;
    }
    if (strcmp(typeString, "TOGGLE") == 0)
    {
        return ButtonAction::Type::TOGGLE;
    }

    return ButtonAction::Type::PRESSED;
}

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

    inputActionElement->SetAttribute("type", toString(buttonType));

    for (Button button : buttons)
    {
        tinyxml2::XMLElement *buttonElement = doc.NewElement("button");
        buttonElement->SetAttribute("code", button.code);
        buttonElement->SetAttribute("source", toString(button.source));

        inputActionElement->InsertEndChild(buttonElement);
    }

    parentElement->InsertEndChild(inputActionElement);
    return *inputActionElement;
}

void ButtonAction::load(tinyxml2::XMLElement *buttonActionElement)
{
    const char *typeString = buttonActionElement->Attribute("type");

    buttonType = toButtonActionType(typeString);

    tinyxml2::XMLElement *buttonElement = buttonActionElement->FirstChildElement("button");
    while (buttonElement)
    {
        int code = buttonElement->IntAttribute("code");
        buttons.push_back({code, toInputSource(buttonElement->Attribute("source"))});

        buttonElement = buttonElement->NextSiblingElement("button");
    }
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
    case (Type::AXES):
        inputActionElement->SetAttribute("type", "AXES");

        for (int i = 0; i < 2; i++)
        {
            tinyxml2::XMLElement *axisElement = doc.NewElement("axis");
            axisElement->SetAttribute("code", axes[i].code);
            axisElement->SetAttribute("source", toString(axes[i].source));

            inputActionElement->InsertEndChild(axisElement);
        }

        break;
    case (Type::BUTTONS):
        inputActionElement->SetAttribute("type", "BUTTONS");

        for (int i = 0; i < 4; i++)
        {
            tinyxml2::XMLElement *axisElement = doc.NewElement("button");
            axisElement->SetAttribute("code", buttons[i].code);
            axisElement->SetAttribute("source", toString(buttons[i].source));

            inputActionElement->InsertEndChild(axisElement);
        }

        break;
    }

    parentElement->InsertEndChild(inputActionElement);
    return *inputActionElement;
}

void AxisAction::load(tinyxml2::XMLElement *axisActionElement)
{
    const char *typeString = axisActionElement->Attribute("type");

    if (strcmp(typeString, "AXES") == 0)
    {
        type = Type::AXES;
        tinyxml2::XMLElement *axisElement = axisActionElement->FirstChildElement("axis");
        int i = 0;
        while (axisElement)
        {
            int code = axisElement->IntAttribute("code");
            axes[i] = {code, toInputSource(axisElement->Attribute("source"))};
            i++;

            axisElement = axisElement->NextSiblingElement("axis");
        }
    }
    if (strcmp(typeString, "BUTTONS") == 0)
    {
        type = Type::BUTTONS;
        tinyxml2::XMLElement *buttonsElement = axisActionElement->FirstChildElement("button");

        int i = 0;
        while (buttonsElement)
        {
            int code = buttonsElement->IntAttribute("code");
            buttons[i] = {code, toInputSource(buttonsElement->Attribute("source"))};

            i++;

            buttonsElement = buttonsElement->NextSiblingElement("button");
        }
    }
}

void AxisAction::renderSettings(int i)
{
    std::string label;

    if (type == Type::AXES)
    {
        for (Axis &axis : axes)
        {
            label = fmt::format("##a{}", i++);
            axis.renderSettings(label);
        }
    }
    if (type == Type::BUTTONS)
    {
        for (Button &button : buttons)
        {
            label = fmt::format("##ab{}", i++);
            button.renderSettings(label);
        }
    }
}

void ButtonAction::renderSettings(int i)
{
    std::string label = fmt::format("##t{}", i++);

    if (ImGui::BeginCombo(label.c_str(), toString(buttonType)))
    {
        for (const Type &type : {Type::HELD, Type::TOGGLE, Type::PRESSED, Type::RELEASED, Type::SHORTCUT})
        {
            bool isSelected = (type == buttonType);

            if (ImGui::Selectable(toString(type), isSelected))
            {
                buttonType = type;
            }

            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    for (Button button : buttons)
    {
        label = fmt::format("##b{}", i++);
        button.renderSettings(label);
    }
}

} // namespace cmx
