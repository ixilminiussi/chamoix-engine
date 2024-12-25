#ifndef CMX_INPUT_ACTION
#define CMX_INPUT_ACTION

// cmx
#include "cmx_inputs.h"

// lib
#include "tinyxml2.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <functional>
#include <initializer_list>
#include <vector>

namespace cmx
{

class InputAction
{
  public:
    InputAction() = default;
    ~InputAction() = default;

    virtual void poll(const class CmxWindow &, float dt) = 0;
    virtual void bind(std::function<void(float, int)> callbackFunction) = 0;
    virtual void bind(std::function<void(float, glm::vec2)> callbackFunction) = 0;

    virtual tinyxml2::XMLElement &save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) = 0;
    virtual void load(tinyxml2::XMLElement *parentElement) = 0;
    virtual void renderSettings() = 0;
};

class ButtonAction : public InputAction
{
  public:
    enum Type
    {
        PRESSED,  // sends when first pressed
        HELD,     // sends as long as toggled
        RELEASED, // sends when released
        SHORTCUT, // TODO: sends when all are held for first time
        TOGGLE,   // sends 1 when pressed, 0 when released
    };

    ButtonAction() : _buttonType{PRESSED} {};

    ButtonAction(Type buttonType, std::initializer_list<Button> buttons)
        : _buttonType{buttonType}, _buttons{buttons} {};

    void poll(const class CmxWindow &, float dt) override;
    void bind(std::function<void(float, int)> callbackFunction) override;
    void bind(std::function<void(float, glm::vec2)> callbackFunction) override;

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) override;
    void load(tinyxml2::XMLElement *parentElement) override;
    void renderSettings() override;

  private:
    std::vector<Button> _buttons;
    std::vector<std::function<void(float, int)>> _functions;

    int _status{GLFW_RELEASE};
    Type _buttonType;
};

class AxisAction : public InputAction
{
  public:
    enum Type
    {
        BUTTONS,
        AXES
    };

    AxisAction() : AxisAction{CMX_AXIS_VOID, CMX_AXIS_VOID} {};

    AxisAction(Button right, Button left, Button up = CMX_BUTTON_VOID, Button down = CMX_BUTTON_VOID)
        : buttons{right, left, up, down}, type{BUTTONS} {};
    AxisAction(Axis vertical, Axis horizontal = CMX_AXIS_VOID) : axes{vertical, horizontal}, type{AXES} {};

    void poll(const class CmxWindow &, float dt) override;
    void bind(std::function<void(float, int)> callbackFunction) override;
    void bind(std::function<void(float, glm::vec2)> callbackFunction) override;

    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) override;
    void load(tinyxml2::XMLElement *parentElement) override;
    void renderSettings() override;

  private:
    Type type;
    Button buttons[4];
    Axis axes[2];

    std::vector<std::function<void(float, glm::vec2)>> functions;

    glm::vec2 value{0.f};
};

} // namespace cmx

#endif
