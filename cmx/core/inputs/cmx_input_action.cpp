#include "cmx_input_action.h"

// cmx
#include "cmx_input_manager.h"
#include "cmx_inputs.h"
#include "cmx_window.h"
#include "imgui.h"

// lib
#include <GLFW/glfw3.h>
#include <IconsMaterialSymbols.h>
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

const char *toString(AxisAction::Type axisType)
{
    switch (axisType)
    {
    case AxisAction::Type::AXES:
        return "AXES";
    case AxisAction::Type::BUTTONS:
        return "BUTTONS";
    }

    return "AXES";
}

void ButtonAction::poll(const Window &window, float dt)
{
    int newStatus = (_buttonType == Type::SHORTCUT && _buttons.size() > 0) ? 1 : 0;

    for (Button &button : _buttons)
    {
        switch (button.source)
        {
        case InputSource::KEYBOARD:
#ifndef NDEBUG
            if (!ImGui::GetIO().WantCaptureKeyboard)
            {
#endif
                button.status = glfwGetKey(window.getGLFWwindow(), button.code);

                if (_buttonType == Type::SHORTCUT)
                {
                    newStatus &= button.status;
                }
                else
                {
                    newStatus |= button.status;
                }
#ifndef NDEBUG
            }
            else
            {
                newStatus = 0;
            }
#endif
            break;
        case InputSource::MOUSE:
#ifndef NDEBUG
            if (!ImGui::GetIO().WantCaptureMouse)
            {
#endif
                button.status = glfwGetMouseButton(window.getGLFWwindow(), button.code);
                if (_buttonType == Type::SHORTCUT)
                {
                    newStatus &= button.status;
                }
                else
                {
                    newStatus |= button.status;
                }
#ifndef NDEBUG
            }
            else
            {
                newStatus = 0;
            }
#endif
            break;
        case InputSource::GAMEPAD:
            // TODO: Gamepad support
            break;
        }
    }

    bool success = false;

    switch (_buttonType)
    {
    case Type::HELD:
        if (newStatus == 1)
        {
            success = true;
        }
        break;

    case Type::PRESSED:
    case Type::SHORTCUT:
        if (newStatus == 1 && _status == 0)
        {
            success = true;
        }
        break;

    case Type::RELEASED:
        if (newStatus == 0 && _status == 1)
        {
            success = true;
        }
        break;
    case Type::TOGGLE:
        if (newStatus != _status)
        {
            success = true;
        }
        break;
    }
    _status = newStatus;

    if (success)
    {
        for (std::function<void(float, int)> func : _functions)
        {
            func(dt, newStatus);
        }
    }
}

void AxisAction::poll(const Window &window, float dt)
{
    switch (_type)
    {
    case Type::BUTTONS:
        for (int i = 0; i < 4; i++)
        {
            if (_buttons[i] == CMX_BUTTON_VOID)
            {
                continue;
            }

            switch (_buttons[i].source)
            {
            case InputSource::KEYBOARD:
#ifndef NDEBUG
                if (!ImGui::GetIO().WantCaptureKeyboard)
#endif
                    _buttons[i].status = glfwGetKey(window.getGLFWwindow(), _buttons[i].code);
                break;
            case InputSource::MOUSE:
#ifndef NDEBUG
                if (!ImGui::GetIO().WantCaptureMouse)
#endif
                    _buttons[i].status = glfwGetMouseButton(window.getGLFWwindow(), _buttons[i].code);
                break;
            case InputSource::GAMEPAD:
                // TODO: Gamepad support
                break;
            }
        }

        _value =
            glm::vec2{float(_buttons[0].status - _buttons[1].status), float(_buttons[2].status - _buttons[3].status)};

        break;
    case Type::AXES:
        for (int i = 0; i < 2; i++)
        {
            if (_axes[i] == CMX_AXIS_VOID)
            {
                continue;
            }
            if (_axes[i] == CMX_MOUSE_AXIS_X_ABSOLUTE)
            {
                double x, y;
                glfwGetCursorPos(window.getGLFWwindow(), &x, &y);
                x *= window.getExtent().width * 0.01f;
                _axes[i].value = float(x);
            }
            if (_axes[i] == CMX_MOUSE_AXIS_Y_ABSOLUTE)
            {
                double x, y;
                glfwGetCursorPos(window.getGLFWwindow(), &x, &y);
                y *= window.getExtent().height * 0.01f;
                _axes[i].value = float(y);
            }
            if (_axes[i] == CMX_MOUSE_AXIS_X_RELATIVE)
            {
                double x, y;
                glfwGetCursorPos(window.getGLFWwindow(), &x, &y);
                x *= window.getExtent().width * 0.01f;
                _axes[i].value = float(x) - _axes[i].absValue;
                _axes[i].absValue = float(x);
            }
            if (_axes[i] == CMX_MOUSE_AXIS_Y_RELATIVE)
            {
                double x, y;
                glfwGetCursorPos(window.getGLFWwindow(), &x, &y);
                y *= window.getExtent().height * 0.01f;
                _axes[i].value = float(y) - _axes[i].absValue;
                _axes[i].absValue = float(y);
            }
            if (_axes[i] == CMX_MOUSE_SCROLL_X_RELATIVE)
            {
                _axes[i].value = float(InputManager::scrollX);
            }
            if (_axes[i] == CMX_MOUSE_SCROLL_Y_RELATIVE)
            {
                _axes[i].value = float(InputManager::scrollY);
            }
        }

        _value = glm::vec2{_axes[0].value, _axes[1].value};

        break;
    }

    if (glm::length(_value) > glm::epsilon<float>())
    {
        glm::vec2 modifierApplied{_value.x * _modifierX, _value.y * _modifierY};
        for (std::function<void(float, glm::vec2)> func : _functions)
        {
            func(dt, modifierApplied);
        }
    }
}

void ButtonAction::bind(std::function<void(float, int)> callbackFunction)
{
    _functions.push_back(callbackFunction);
}

void ButtonAction::bind(std::function<void(float, glm::vec2)> callbackFunction)
{
    spdlog::critical("ButtonAction: can only be bound to std::function<void(float, int)>");
    std::exit(EXIT_FAILURE);
}

void ButtonAction::unbindAll()
{
    _functions.clear();
}

tinyxml2::XMLElement &ButtonAction::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement)
{
    tinyxml2::XMLElement *inputActionElement = doc.NewElement("buttonAction");

    inputActionElement->SetAttribute("type", toString(_buttonType));

    for (Button button : _buttons)
    {
        tinyxml2::XMLElement *buttonElement = doc.NewElement("button");
        buttonElement->SetAttribute("code", button.code);
        buttonElement->SetAttribute("source", toString(button.source));
        buttonElement->SetAttribute("id", button.id);

        inputActionElement->InsertEndChild(buttonElement);
    }

    parentElement->InsertEndChild(inputActionElement);
    return *inputActionElement;
}

void ButtonAction::load(tinyxml2::XMLElement *buttonActionElement)
{
    const char *typeString = buttonActionElement->Attribute("type");

    _buttonType = toButtonActionType(typeString);

    tinyxml2::XMLElement *buttonElement = buttonActionElement->FirstChildElement("button");
    while (buttonElement)
    {
        int code = buttonElement->IntAttribute("code");
        short unsigned int id = buttonElement->UnsignedAttribute("id");
        _buttons.push_back({code, toInputSource(buttonElement->Attribute("source")), id});

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
    _functions.push_back(callbackFunction);
}

tinyxml2::XMLElement &AxisAction::save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement)
{
    tinyxml2::XMLElement *inputActionElement = doc.NewElement("axisAction");
    inputActionElement->SetAttribute("modifierX", _modifierX);
    inputActionElement->SetAttribute("modifierY", _modifierY);

    switch (_type)
    {
    case (Type::AXES):
        inputActionElement->SetAttribute("type", "AXES");

        for (int i = 0; i < 2; i++)
        {
            tinyxml2::XMLElement *axisElement = doc.NewElement("axis");
            axisElement->SetAttribute("code", _axes[i].code);
            axisElement->SetAttribute("source", toString(_axes[i].source));
            axisElement->SetAttribute("id", _axes[i].id);

            inputActionElement->InsertEndChild(axisElement);
        }

        break;
    case (Type::BUTTONS):
        inputActionElement->SetAttribute("type", "BUTTONS");

        for (int i = 0; i < 4; i++)
        {
            tinyxml2::XMLElement *axisElement = doc.NewElement("button");
            axisElement->SetAttribute("code", _buttons[i].code);
            axisElement->SetAttribute("source", toString(_buttons[i].source));
            axisElement->SetAttribute("id", _buttons[i].id);

            inputActionElement->InsertEndChild(axisElement);
        }

        break;
    }

    parentElement->InsertEndChild(inputActionElement);
    return *inputActionElement;
}

void AxisAction::unbindAll()
{
    _functions.clear();
}

void AxisAction::load(tinyxml2::XMLElement *axisActionElement)
{
    const char *typeString = axisActionElement->Attribute("type");
    _modifierX = axisActionElement->FloatAttribute("modifierX");
    _modifierY = axisActionElement->FloatAttribute("modifierY");

    if (strcmp(typeString, "AXES") == 0)
    {
        _type = Type::AXES;
        tinyxml2::XMLElement *axisElement = axisActionElement->FirstChildElement("axis");
        int i = 0;
        while (axisElement)
        {
            int code = axisElement->IntAttribute("code");
            short unsigned int id = axisElement->UnsignedAttribute("id");
            _axes[i] = {code, toInputSource(axisElement->Attribute("source")), id};
            i++;

            axisElement = axisElement->NextSiblingElement("axis");
        }
    }
    if (strcmp(typeString, "BUTTONS") == 0)
    {
        _type = Type::BUTTONS;
        tinyxml2::XMLElement *buttonsElement = axisActionElement->FirstChildElement("button");

        int i = 0;
        while (buttonsElement)
        {
            int code = buttonsElement->IntAttribute("code");
            _buttons[i] = {code, toInputSource(buttonsElement->Attribute("source"))};

            i++;

            buttonsElement = buttonsElement->NextSiblingElement("button");
        }
    }
}

void AxisAction::editor()
{
    int i = 0;
    std::string label;

    ImGui::SetNextItemWidth(100);
    if (ImGui::BeginCombo("type", toString(_type)))
    {
        for (const Type &axisType : {Type::BUTTONS, Type::AXES})
        {
            bool isSelected = (axisType == _type);

            if (ImGui::Selectable(toString(axisType), isSelected))
            {
                _type = axisType;
            }

            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

    if (_type == Type::AXES)
    {
        label = fmt::format("X##a{}", i++);
        _axes[0].editor(label);
        label = fmt::format("modifier##a{}", i++);
        ImGui::InputFloat(label.c_str(), &_modifierX);
        label = fmt::format("Y##a{}", i++);
        _axes[1].editor(label);
        label = fmt::format("modifier##a{}", i++);
        ImGui::InputFloat(label.c_str(), &_modifierY);
    }
    if (_type == Type::BUTTONS)
    {
        label = fmt::format("R##a{}", i++);
        _buttons[0].editor(label);
        label = fmt::format("L##a{}", i++);
        _buttons[1].editor(label);
        label = fmt::format("modifier##a{}", i++);
        ImGui::InputFloat(label.c_str(), &_modifierX);
        label = fmt::format("U##a{}", i++);
        _buttons[2].editor(label);
        label = fmt::format("D##a{}", i++);
        _buttons[3].editor(label);
        label = fmt::format("modifier##a{}", i++);
        ImGui::InputFloat(label.c_str(), &_modifierY);
    }
}

void ButtonAction::editor()
{
    int i = 0;
    std::string label;

    ImGui::SetNextItemWidth(100);
    if (ImGui::BeginCombo("mode", toString(_buttonType)))
    {
        for (const Type &type : {Type::HELD, Type::TOGGLE, Type::PRESSED, Type::RELEASED, Type::SHORTCUT})
        {
            bool isSelected = (type == _buttonType);

            if (ImGui::Selectable(toString(type), isSelected))
            {
                _buttonType = type;
            }

            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    auto it = _buttons.begin();
    while (it != _buttons.end())
    {
        label = fmt::format("##b{}", i++);
        it->editor(label);

        ImGui::SameLine();
        label = fmt::format(ICON_MS_REMOVE "##r{}", i++);
        if (ImGui::Button(label.c_str()))
        {
            _buttons.erase(it);
        }
        else
        {
            it++;
        }
    }

    label = fmt::format(ICON_MS_ADD "##p{}", i++);
    ImGui::SameLine();
    if (ImGui::Button(label.c_str()))
    {
        _buttons.push_back(CMX_BUTTON_VOID);
    }
}

} // namespace cmx
