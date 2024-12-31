#include "cmx_input_manager.h"

// cmx
#include "cmx_input_action.h"
#include "cmx_window.h"
#include "imgui.h"

// lib
#include "IconsMaterialSymbols.h"
#include "tinyxml2.h"
#include <GLFW/glfw3.h>

// std
#include <cstdlib>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <unordered_map>

namespace cmx
{

InputManager::InputManager(CmxWindow &window, const std::string &filepath)
    : _window{window}, _inputDictionary{}, _filepath{filepath}
{
    _gamepadDetected = glfwJoystickPresent(GLFW_JOYSTICK_1);
    glfwSetInputMode(window.getGLFWwindow(), GLFW_STICKY_KEYS, GLFW_TRUE);
}

InputManager::~InputManager()
{
    for (auto &pair : _inputDictionary)
    {
        delete pair.second;
    }
    _inputDictionary.clear();
}

void InputManager::addInput(const std::string &name, InputAction *newInput)
{
    try
    {
        auto attempt = _inputDictionary.at(name);
    }
    catch (const std::out_of_range &e)
    {
        _inputDictionary[name] = newInput;
        spdlog::info("InputManager: New input '{0}' added", name.c_str());
        return;
    }
    spdlog::error("InputManager: Attempt at rebinding existing input '{0}'", name.c_str());
}

void InputManager::pollEvents(float dt)
{
    glfwPollEvents();

    for (auto &[name, input] : _inputDictionary)
    {
        if (auto button = static_cast<ButtonAction *>(input))
        {
            button->poll(_window, dt);
        }
        if (auto button = static_cast<AxisAction *>(input))
        {
            button->poll(_window, dt);
        }
    }
}

void InputManager::setMouseCapture(bool b)
{
    (b) ? glfwSetInputMode(_window.getGLFWwindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED)
        : glfwSetInputMode(_window.getGLFWwindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void InputManager::save()
{
    saveAs(_filepath);
}

void InputManager::saveAs(const std::string &customFilepath)
{
    spdlog::info("InputManager: saving state to {0}", customFilepath);

    tinyxml2::XMLDocument doc;
    doc.InsertFirstChild(doc.NewDeclaration());

    tinyxml2::XMLElement *inputManagerElement = doc.NewElement("inputs");

    for (const auto &pair : _inputDictionary)
    {
        tinyxml2::XMLElement &inputActionElement = pair.second->save(doc, inputManagerElement);
        inputActionElement.SetAttribute("name", pair.first.c_str());
    }

    doc.InsertEndChild(inputManagerElement);

    if (doc.SaveFile(customFilepath.c_str()) != tinyxml2::XML_SUCCESS)
    {
        spdlog::error("FILE SAVING: {0}", doc.ErrorStr());
    };

    spdlog::info("InputManager: saving success!", _filepath);
}

void InputManager::load()
{
    tinyxml2::XMLDocument doc;

    if (doc.LoadFile(_filepath.c_str()) == tinyxml2::XML_SUCCESS)
    {
        spdlog::info("InputManager: Loading input manager from {0}...", _filepath);

        tinyxml2::XMLElement *rootElement = doc.RootElement();
        if (tinyxml2::XMLElement *inputsElement = doc.FirstChildElement("inputs"))
        {
            tinyxml2::XMLElement *buttonActionElement = inputsElement->FirstChildElement("buttonAction");
            while (buttonActionElement)
            {
                ButtonAction *buttonAction = new ButtonAction{};
                buttonAction->load(buttonActionElement);
                addInput(buttonActionElement->Attribute("name"), buttonAction);

                buttonActionElement = buttonActionElement->NextSiblingElement("buttonAction");
            }

            tinyxml2::XMLElement *axisActionElement = inputsElement->FirstChildElement("axisAction");
            while (axisActionElement)
            {
                AxisAction *axisAction = new AxisAction{};
                axisAction->load(axisActionElement);
                addInput(axisActionElement->Attribute("name"), axisAction);

                axisActionElement = axisActionElement->NextSiblingElement("axisAction");
            }
        }

        spdlog::info("InputManager: Successfully loaded input manager!");
    }
}

void InputManager::editor()
{
    int i = 0;

    auto it = _inputDictionary.begin();
    while (it != _inputDictionary.end())
    {
        ImGui::PushID(i++);
        if (ImGui::CollapsingHeader(it->first.c_str()))
        {
            if (ImGui::Button(ICON_MS_DELETE " delete"))
            {
                it = _inputDictionary.erase(it);
            }
            else
            {
                it->second->editor();
            }
        }
        it++;

        ImGui::PopID();
    }

    ImGui::Separator();
    static char buffer[100] = "";
    ImGui::SetNextItemWidth(150);
    ImGui::InputText("##gfd", buffer, 100);
    ImGui::SameLine();
    if (ImGui::Button("Axis"))
    {
        addInput(std::string(buffer), new AxisAction{});
    }
    ImGui::SameLine();
    if (ImGui::Button("Button"))
    {
        addInput(std::string(buffer), new ButtonAction{});
    }

    ImGui::Separator();
    if (ImGui::Button("Apply"))
    {
        save();
    }
}

} // namespace cmx
