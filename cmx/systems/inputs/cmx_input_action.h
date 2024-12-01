#pragma once

// lib
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// std
#include <functional>
#include <initializer_list>
#include <vector>

namespace cmx
{

enum InputSource
{
    GAMEPAD,
    MOUSE,
    KEYBOARD
};

struct Button
{

    int code;
    InputSource source;
    int status{0};
};

inline bool operator==(const Button &a, const Button &b)
{
    return (a.code == b.code && a.source == b.source);
}

struct Axis
{
    int code;
    InputSource source;
    float value{0.f};
    float absValue{0.f};
};

inline bool operator==(const Axis &a, const Axis &b)
{
    return (a.code == b.code && a.source == b.source);
}

inline Button CMX_BUTTON_VOID{-1, InputSource::KEYBOARD};
inline Axis CMX_AXIS_VOID{-1, InputSource::KEYBOARD};
inline Axis CMX_MOUSE_AXIS_X_ABSOLUTE{0, InputSource::MOUSE};
inline Axis CMX_MOUSE_AXIS_Y_ABSOLUTE{1, InputSource::MOUSE};
inline Axis CMX_MOUSE_AXIS_X_RELATIVE{0, InputSource::MOUSE};
inline Axis CMX_MOUSE_AXIS_Y_RELATIVE{1, InputSource::MOUSE};

class InputAction
{
  public:
    InputAction() = default;
    ~InputAction() = default;

    virtual void poll(const class CmxWindow &, float dt) = 0;
    virtual void bind(std::function<void(float)> callbackFunction) = 0;
    virtual void bind(std::function<void(float, glm::vec2)> callbackFunction) = 0;
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
        TOGGLE,   // TODO: sends 1 when pressed, -1 when released
    };

    ButtonAction(Type buttonType, std::initializer_list<Button> buttons) : buttonType{buttonType}, buttons{buttons} {};

    void poll(const class CmxWindow &, float dt) override;
    void bind(std::function<void(float)> callbackFunction) override;
    void bind(std::function<void(float, glm::vec2)> callbackFunction) override;

  private:
    std::vector<Button> buttons;
    std::vector<std::function<void(float)>> functions;

    int status{GLFW_RELEASE};
    Type buttonType;
};

class AxisAction : public InputAction
{
    enum Type
    {
        BUTTONS,
        AXES
    };

  public:
    AxisAction(Button right, Button left, Button up = CMX_BUTTON_VOID, Button down = CMX_BUTTON_VOID)
        : buttons{right, left, up, down}, type{BUTTONS} {};
    AxisAction(Axis vertical, Axis horizontal = CMX_AXIS_VOID) : axes{vertical, horizontal}, type{AXES} {};

    void poll(const class CmxWindow &, float dt) override;
    void bind(std::function<void(float)> callbackFunction) override;
    void bind(std::function<void(float, glm::vec2)> callbackFunction) override;

  private:
    Type type;
    Button buttons[4];
    Axis axes[2];

    std::vector<std::function<void(float, glm::vec2)>> functions;

    glm::vec2 value{0.f};
};

inline Button CMX_KEY_SPACE{GLFW_KEY_SPACE, InputSource::KEYBOARD};
inline Button CMX_KEY_APOSTROPHE{GLFW_KEY_APOSTROPHE, InputSource::KEYBOARD};
inline Button CMX_KEY_COMMA{GLFW_KEY_COMMA, InputSource::KEYBOARD};
inline Button CMX_KEY_MINUS{GLFW_KEY_MINUS, InputSource::KEYBOARD};
inline Button CMX_KEY_PERIOD{GLFW_KEY_PERIOD, InputSource::KEYBOARD};
inline Button CMX_KEY_SLASH{GLFW_KEY_SLASH, InputSource::KEYBOARD};
inline Button CMX_KEY_0{GLFW_KEY_0, InputSource::KEYBOARD};
inline Button CMX_KEY_1{GLFW_KEY_1, InputSource::KEYBOARD};
inline Button CMX_KEY_2{GLFW_KEY_2, InputSource::KEYBOARD};
inline Button CMX_KEY_3{GLFW_KEY_3, InputSource::KEYBOARD};
inline Button CMX_KEY_4{GLFW_KEY_4, InputSource::KEYBOARD};
inline Button CMX_KEY_5{GLFW_KEY_5, InputSource::KEYBOARD};
inline Button CMX_KEY_6{GLFW_KEY_6, InputSource::KEYBOARD};
inline Button CMX_KEY_7{GLFW_KEY_7, InputSource::KEYBOARD};
inline Button CMX_KEY_8{GLFW_KEY_8, InputSource::KEYBOARD};
inline Button CMX_KEY_9{GLFW_KEY_9, InputSource::KEYBOARD};
inline Button CMX_KEY_SEMICOLON{GLFW_KEY_SEMICOLON, InputSource::KEYBOARD};
inline Button CMX_KEY_EQUAL{GLFW_KEY_EQUAL, InputSource::KEYBOARD};
inline Button CMX_KEY_A{GLFW_KEY_A, InputSource::KEYBOARD};
inline Button CMX_KEY_B{GLFW_KEY_B, InputSource::KEYBOARD};
inline Button CMX_KEY_C{GLFW_KEY_C, InputSource::KEYBOARD};
inline Button CMX_KEY_D{GLFW_KEY_D, InputSource::KEYBOARD};
inline Button CMX_KEY_E{GLFW_KEY_E, InputSource::KEYBOARD};
inline Button CMX_KEY_F{GLFW_KEY_F, InputSource::KEYBOARD};
inline Button CMX_KEY_H{GLFW_KEY_H, InputSource::KEYBOARD};
inline Button CMX_KEY_I{GLFW_KEY_I, InputSource::KEYBOARD};
inline Button CMX_KEY_J{GLFW_KEY_J, InputSource::KEYBOARD};
inline Button CMX_KEY_K{GLFW_KEY_K, InputSource::KEYBOARD};
inline Button CMX_KEY_L{GLFW_KEY_L, InputSource::KEYBOARD};
inline Button CMX_KEY_M{GLFW_KEY_M, InputSource::KEYBOARD};
inline Button CMX_KEY_N{GLFW_KEY_N, InputSource::KEYBOARD};
inline Button CMX_KEY_O{GLFW_KEY_O, InputSource::KEYBOARD};
inline Button CMX_KEY_P{GLFW_KEY_P, InputSource::KEYBOARD};
inline Button CMX_KEY_Q{GLFW_KEY_Q, InputSource::KEYBOARD};
inline Button CMX_KEY_R{GLFW_KEY_R, InputSource::KEYBOARD};
inline Button CMX_KEY_S{GLFW_KEY_S, InputSource::KEYBOARD};
inline Button CMX_KEY_T{GLFW_KEY_T, InputSource::KEYBOARD};
inline Button CMX_KEY_U{GLFW_KEY_U, InputSource::KEYBOARD};
inline Button CMX_KEY_V{GLFW_KEY_V, InputSource::KEYBOARD};
inline Button CMX_KEY_W{GLFW_KEY_W, InputSource::KEYBOARD};
inline Button CMX_KEY_X{GLFW_KEY_X, InputSource::KEYBOARD};
inline Button CMX_KEY_Y{GLFW_KEY_Y, InputSource::KEYBOARD};
inline Button CMX_KEY_Z{GLFW_KEY_Z, InputSource::KEYBOARD};
inline Button CMX_KEY_LEFT_BRACKET{GLFW_KEY_LEFT_BRACKET, InputSource::KEYBOARD};
inline Button CMX_KEY_BACKSLASH{GLFW_KEY_BACKSLASH, InputSource::KEYBOARD};
inline Button CMX_KEY_RIGHT_BRACKET{GLFW_KEY_RIGHT_BRACKET, InputSource::KEYBOARD};
inline Button CMX_KEY_GRAVE_ACCENT{GLFW_KEY_GRAVE_ACCENT, InputSource::KEYBOARD};
inline Button CMX_KEY_WORLD_1{GLFW_KEY_WORLD_1, InputSource::KEYBOARD};
inline Button CMX_KEY_WORLD_2{GLFW_KEY_WORLD_2, InputSource::KEYBOARD};
inline Button CMX_KEY_ESCAPE{GLFW_KEY_ESCAPE, InputSource::KEYBOARD};
inline Button CMX_KEY_ENTER{GLFW_KEY_ENTER, InputSource::KEYBOARD};
inline Button CMX_KEY_TAB{GLFW_KEY_TAB, InputSource::KEYBOARD};
inline Button CMX_KEY_BACKSPACE{GLFW_KEY_BACKSPACE, InputSource::KEYBOARD};
inline Button CMX_KEY_INSERT{GLFW_KEY_INSERT, InputSource::KEYBOARD};
inline Button CMX_KEY_DELETE{GLFW_KEY_DELETE, InputSource::KEYBOARD};
inline Button CMX_KEY_RIGHT{GLFW_KEY_RIGHT, InputSource::KEYBOARD};
inline Button CMX_KEY_LEFT{GLFW_KEY_LEFT, InputSource::KEYBOARD};
inline Button CMX_KEY_DOWN{GLFW_KEY_DOWN, InputSource::KEYBOARD};
inline Button CMX_KEY_UP{GLFW_KEY_UP, InputSource::KEYBOARD};
inline Button CMX_KEY_PAGE_UP{GLFW_KEY_PAGE_UP, InputSource::KEYBOARD};
inline Button CMX_KEY_PAGE_DOWN{GLFW_KEY_PAGE_DOWN, InputSource::KEYBOARD};
inline Button CMX_KEY_HOME{GLFW_KEY_HOME, InputSource::KEYBOARD};
inline Button CMX_KEY_END{GLFW_KEY_END, InputSource::KEYBOARD};
inline Button CMX_KEY_CAPS_LOCK{GLFW_KEY_CAPS_LOCK, InputSource::KEYBOARD};
inline Button CMX_KEY_SCROLL_LOCK{GLFW_KEY_SCROLL_LOCK, InputSource::KEYBOARD};
inline Button CMX_KEY_NUM_LOCK{GLFW_KEY_NUM_LOCK, InputSource::KEYBOARD};
inline Button CMX_KEY_PRINT_SCREEN{GLFW_KEY_PRINT_SCREEN, InputSource::KEYBOARD};
inline Button CMX_KEY_PAUSE{GLFW_KEY_PAUSE, InputSource::KEYBOARD};
inline Button CMX_KEY_F1{GLFW_KEY_F1, InputSource::KEYBOARD};
inline Button CMX_KEY_F2{GLFW_KEY_F2, InputSource::KEYBOARD};
inline Button CMX_KEY_F3{GLFW_KEY_F3, InputSource::KEYBOARD};
inline Button CMX_KEY_F4{GLFW_KEY_F4, InputSource::KEYBOARD};
inline Button CMX_KEY_F5{GLFW_KEY_F5, InputSource::KEYBOARD};
inline Button CMX_KEY_F6{GLFW_KEY_F6, InputSource::KEYBOARD};
inline Button CMX_KEY_F7{GLFW_KEY_F7, InputSource::KEYBOARD};
inline Button CMX_KEY_F8{GLFW_KEY_F8, InputSource::KEYBOARD};
inline Button CMX_KEY_F9{GLFW_KEY_F9, InputSource::KEYBOARD};
inline Button CMX_KEY_F10{GLFW_KEY_F10, InputSource::KEYBOARD};
inline Button CMX_KEY_F11{GLFW_KEY_F11, InputSource::KEYBOARD};
inline Button CMX_KEY_F12{GLFW_KEY_F12, InputSource::KEYBOARD};
inline Button CMX_KEY_F13{GLFW_KEY_F13, InputSource::KEYBOARD};
inline Button CMX_KEY_F14{GLFW_KEY_F14, InputSource::KEYBOARD};
inline Button CMX_KEY_F15{GLFW_KEY_F15, InputSource::KEYBOARD};
inline Button CMX_KEY_F16{GLFW_KEY_F16, InputSource::KEYBOARD};
inline Button CMX_KEY_F17{GLFW_KEY_F17, InputSource::KEYBOARD};
inline Button CMX_KEY_F18{GLFW_KEY_F18, InputSource::KEYBOARD};
inline Button CMX_KEY_F19{GLFW_KEY_F19, InputSource::KEYBOARD};
inline Button CMX_KEY_F20{GLFW_KEY_F20, InputSource::KEYBOARD};
inline Button CMX_KEY_F21{GLFW_KEY_F21, InputSource::KEYBOARD};
inline Button CMX_KEY_F22{GLFW_KEY_F22, InputSource::KEYBOARD};
inline Button CMX_KEY_F23{GLFW_KEY_F23, InputSource::KEYBOARD};
inline Button CMX_KEY_F24{GLFW_KEY_F24, InputSource::KEYBOARD};
inline Button CMX_KEY_F25{GLFW_KEY_F25, InputSource::KEYBOARD};
inline Button CMX_KEY_KP_0{GLFW_KEY_KP_0, InputSource::KEYBOARD};
inline Button CMX_KEY_KP_1{GLFW_KEY_KP_1, InputSource::KEYBOARD};
inline Button CMX_KEY_KP_2{GLFW_KEY_KP_2, InputSource::KEYBOARD};
inline Button CMX_KEY_KP_3{GLFW_KEY_KP_3, InputSource::KEYBOARD};
inline Button CMX_KEY_KP_4{GLFW_KEY_KP_4, InputSource::KEYBOARD};
inline Button CMX_KEY_KP_5{GLFW_KEY_KP_5, InputSource::KEYBOARD};
inline Button CMX_KEY_KP_6{GLFW_KEY_KP_6, InputSource::KEYBOARD};
inline Button CMX_KEY_KP_7{GLFW_KEY_KP_7, InputSource::KEYBOARD};
inline Button CMX_KEY_KP_8{GLFW_KEY_KP_8, InputSource::KEYBOARD};
inline Button CMX_KEY_KP_9{GLFW_KEY_KP_9, InputSource::KEYBOARD};
inline Button CMX_KEY_KP_DECIMAL{GLFW_KEY_KP_DECIMAL, InputSource::KEYBOARD};
inline Button CMX_KEY_KP_DIVIDE{GLFW_KEY_KP_DIVIDE, InputSource::KEYBOARD};
inline Button CMX_KEY_KP_MULTIPLY{GLFW_KEY_KP_MULTIPLY, InputSource::KEYBOARD};
inline Button CMX_KEY_KP_SUBTRACT{GLFW_KEY_KP_SUBTRACT, InputSource::KEYBOARD};
inline Button CMX_KEY_KP_ADD{GLFW_KEY_KP_ADD, InputSource::KEYBOARD};
inline Button CMX_KEY_KP_ENTER{GLFW_KEY_KP_ENTER, InputSource::KEYBOARD};
inline Button CMX_KEY_KP_EQUAL{GLFW_KEY_KP_EQUAL, InputSource::KEYBOARD};
inline Button CMX_KEY_LEFT_SHIFT{GLFW_KEY_LEFT_SHIFT, InputSource::KEYBOARD};
inline Button CMX_KEY_LEFT_CONTROL{GLFW_KEY_LEFT_CONTROL, InputSource::KEYBOARD};
inline Button CMX_KEY_LEFT_ALT{GLFW_KEY_KP_3, InputSource::KEYBOARD};
inline Button CMX_KEY_LEFT_SUPER{GLFW_KEY_LEFT_SUPER, InputSource::KEYBOARD};
inline Button CMX_KEY_RIGHT_SHIFT{GLFW_KEY_RIGHT_SHIFT, InputSource::KEYBOARD};
inline Button CMX_KEY_RIGHT_CONTROL{GLFW_KEY_RIGHT_CONTROL, InputSource::KEYBOARD};
inline Button CMX_KEY_RIGHT_ALT{GLFW_KEY_RIGHT_ALT, InputSource::KEYBOARD};
inline Button CMX_KEY_RIGHT_SUPER{GLFW_KEY_RIGHT_SUPER, InputSource::KEYBOARD};
inline Button CMX_KEY_MENU{GLFW_KEY_MENU, InputSource::KEYBOARD};

inline Button CMX_MOUSE_BUTTON_1{GLFW_MOUSE_BUTTON_1, InputSource::MOUSE};
inline Button CMX_MOUSE_BUTTON_2{GLFW_MOUSE_BUTTON_2, InputSource::MOUSE};
inline Button CMX_MOUSE_BUTTON_3{GLFW_MOUSE_BUTTON_3, InputSource::MOUSE};
inline Button CMX_MOUSE_BUTTON_4{GLFW_MOUSE_BUTTON_4, InputSource::MOUSE};
inline Button CMX_MOUSE_BUTTON_5{GLFW_MOUSE_BUTTON_5, InputSource::MOUSE};
inline Button CMX_MOUSE_BUTTON_6{GLFW_MOUSE_BUTTON_6, InputSource::MOUSE};
inline Button CMX_MOUSE_BUTTON_7{GLFW_MOUSE_BUTTON_7, InputSource::MOUSE};
inline Button CMX_MOUSE_BUTTON_8{GLFW_MOUSE_BUTTON_8, InputSource::MOUSE};
inline Button CMX_MOUSE_BUTTON_LAST{GLFW_MOUSE_BUTTON_LAST, InputSource::MOUSE};
inline Button CMX_MOUSE_BUTTON_LEFT{GLFW_MOUSE_BUTTON_LEFT, InputSource::MOUSE};
inline Button CMX_MOUSE_BUTTON_RIGHT{GLFW_MOUSE_BUTTON_RIGHT, InputSource::MOUSE};
inline Button CMX_MOUSE_BUTTON_MIDDLE{GLFW_MOUSE_BUTTON_MIDDLE, InputSource::MOUSE};

// inline Button CMX_GAMEPAD_BUTTON_A{GLFW_GAMEPAD_BUTTON_A, InputSource::GAMEPAD};
// inline Button CMX_GAMEPAD_BUTTON_B{GLFW_GAMEPAD_BUTTON_B, InputSource::GAMEPAD};
// inline Button CMX_GAMEPAD_BUTTON_X{GLFW_GAMEPAD_BUTTON_X, InputSource::GAMEPAD};
// inline Button CMX_GAMEPAD_BUTTON_Y{GLFW_GAMEPAD_BUTTON_Y, InputSource::GAMEPAD};
// inline Button CMX_GAMEPAD_BUTTON_CROSS{GLFW_GAMEPAD_BUTTON_CROSS, InputSource::GAMEPAD};
// inline Button CMX_GAMEPAD_BUTTON_CIRCLE{GLFW_GAMEPAD_BUTTON_CIRCLE, InputSource::GAMEPAD};
// inline Button CMX_GAMEPAD_BUTTON_SQUARE{GLFW_GAMEPAD_BUTTON_SQUARE, InputSource::GAMEPAD};
// inline Button CMX_GAMEPAD_BUTTON_TRIANGLE{GLFW_GAMEPAD_BUTTON_TRIANGLE, InputSource::GAMEPAD};
// inline Button CMX_GAMEPAD_BUTTON_LEFT_BUMPER{GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, InputSource::GAMEPAD};
// inline Button CMX_GAMEPAD_BUTTON_RIGHT_BUMPER{GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, InputSource::GAMEPAD};
// inline Button CMX_GAMEPAD_BUTTON_BACK{GLFW_GAMEPAD_BUTTON_BACK, InputSource::GAMEPAD};
// inline Button CMX_GAMEPAD_BUTTON_START{GLFW_GAMEPAD_BUTTON_START, InputSource::GAMEPAD};
// inline Button CMX_GAMEPAD_BUTTON_GUIDE{GLFW_GAMEPAD_BUTTON_GUIDE, InputSource::GAMEPAD};
// inline Button CMX_GAMEPAD_BUTTON_LEFT_THUMB{GLFW_GAMEPAD_BUTTON_LEFT_THUMB, InputSource::GAMEPAD};
// inline Button CMX_GAMEPAD_BUTTON_RIGHT_THUMB{GLFW_GAMEPAD_BUTTON_RIGHT_THUMB, InputSource::GAMEPAD};
// inline Button CMX_GAMEPAD_BUTTON_DPAD_UP{GLFW_GAMEPAD_BUTTON_DPAD_UP, InputSource::GAMEPAD};
// inline Button CMX_GAMEPAD_BUTTON_DPAD_RIGHT{GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, InputSource::GAMEPAD};
// inline Button CMX_GAMEPAD_BUTTON_DPAD_DOWN{GLFW_GAMEPAD_BUTTON_DPAD_DOWN, InputSource::GAMEPAD};
// inline Button CMX_GAMEPAD_BUTTON_DPAD_LEFT{GLFW_GAMEPAD_BUTTON_DPAD_LEFT, InputSource::GAMEPAD};

} // namespace cmx
