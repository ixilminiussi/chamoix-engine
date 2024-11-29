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

enum CmxInputSource
{
    GAMEPAD,
    MOUSE,
    KEYBOARD
};

struct CmxButton
{

    int code;
    CmxInputSource source;
    int status{0};
};

struct CmxAxis
{
    int code;
    CmxInputSource source;
    float value{0.f};
};

inline CmxButton CMX_BUTTON_VOID{-1, CmxInputSource::KEYBOARD};
inline CmxAxis CMX_AXIS_VOID{-1, CmxInputSource::KEYBOARD};

class CmxInputAction
{
  public:
    CmxInputAction() = default;
    ~CmxInputAction() = default;

    virtual void poll(const class CmxWindow &, float dt) = 0;
    virtual void bind(std::function<void(float)> callbackFunction) = 0;
    virtual void bind(std::function<void(float, glm::vec2)> callbackFunction) = 0;
};

class CmxButtonAction : public CmxInputAction
{
  public:
    enum Type
    {
        PRESSED,
        HELD,
        RELEASED,
    };

    CmxButtonAction(Type buttonType, std::initializer_list<CmxButton> buttons)
        : buttonType{buttonType}, buttons{buttons} {};

    void poll(const class CmxWindow &, float dt) override;
    void bind(std::function<void(float)> callbackFunction) override;
    void bind(std::function<void(float, glm::vec2)> callbackFunction) override;

  private:
    std::vector<CmxButton> buttons;
    std::vector<std::function<void(float)>> functions;

    int status{GLFW_RELEASE};
    Type buttonType;
};

class CmxAxisAction : public CmxInputAction
{
    enum Type
    {
        BUTTONS,
        AXES
    };

  public:
    CmxAxisAction(CmxButton up, CmxButton down, CmxButton left = CMX_BUTTON_VOID, CmxButton right = CMX_BUTTON_VOID)
        : buttons{up, down, left, right}, type{BUTTONS} {};
    CmxAxisAction(CmxAxis vertical, CmxAxis horizontal = CMX_AXIS_VOID) : axes{vertical, horizontal}, type{AXES} {};

    void poll(const class CmxWindow &, float dt) override;
    void bind(std::function<void(float)> callbackFunction) override;
    void bind(std::function<void(float, glm::vec2)> callbackFunction) override;

  private:
    Type type;
    CmxButton buttons[4];
    CmxAxis axes[2];

    std::vector<std::function<void(float, glm::vec2)>> functions;

    glm::vec2 value{0.f};
};

inline CmxButton CMX_KEY_SPACE{GLFW_KEY_SPACE, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_APOSTROPHE{GLFW_KEY_APOSTROPHE, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_COMMA{GLFW_KEY_COMMA, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_MINUS{GLFW_KEY_MINUS, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_PERIOD{GLFW_KEY_PERIOD, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_SLASH{GLFW_KEY_SLASH, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_0{GLFW_KEY_0, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_1{GLFW_KEY_1, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_2{GLFW_KEY_2, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_3{GLFW_KEY_3, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_4{GLFW_KEY_4, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_5{GLFW_KEY_5, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_6{GLFW_KEY_6, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_7{GLFW_KEY_7, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_8{GLFW_KEY_8, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_9{GLFW_KEY_9, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_SEMICOLON{GLFW_KEY_SEMICOLON, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_EQUAL{GLFW_KEY_EQUAL, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_A{GLFW_KEY_A, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_B{GLFW_KEY_B, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_C{GLFW_KEY_C, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_D{GLFW_KEY_D, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_E{GLFW_KEY_E, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_F{GLFW_KEY_F, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_H{GLFW_KEY_H, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_I{GLFW_KEY_I, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_J{GLFW_KEY_J, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_K{GLFW_KEY_K, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_L{GLFW_KEY_L, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_M{GLFW_KEY_M, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_N{GLFW_KEY_N, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_O{GLFW_KEY_O, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_P{GLFW_KEY_P, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_Q{GLFW_KEY_Q, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_R{GLFW_KEY_R, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_S{GLFW_KEY_S, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_T{GLFW_KEY_T, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_U{GLFW_KEY_U, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_V{GLFW_KEY_V, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_W{GLFW_KEY_W, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_X{GLFW_KEY_X, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_Y{GLFW_KEY_Y, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_Z{GLFW_KEY_Z, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_LEFT_BRACKET{GLFW_KEY_LEFT_BRACKET, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_BACKSLASH{GLFW_KEY_BACKSLASH, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_RIGHT_BRACKET{GLFW_KEY_RIGHT_BRACKET, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_GRAVE_ACCENT{GLFW_KEY_GRAVE_ACCENT, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_WORLD_1{GLFW_KEY_WORLD_1, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_WORLD_2{GLFW_KEY_WORLD_2, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_ESCAPE{GLFW_KEY_ESCAPE, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_ENTER{GLFW_KEY_ENTER, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_TAB{GLFW_KEY_TAB, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_BACKSPACE{GLFW_KEY_BACKSPACE, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_INSERT{GLFW_KEY_INSERT, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_DELETE{GLFW_KEY_DELETE, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_RIGHT{GLFW_KEY_RIGHT, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_LEFT{GLFW_KEY_LEFT, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_DOWN{GLFW_KEY_DOWN, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_UP{GLFW_KEY_UP, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_PAGE_UP{GLFW_KEY_PAGE_UP, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_PAGE_DOWN{GLFW_KEY_PAGE_DOWN, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_HOME{GLFW_KEY_HOME, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_END{GLFW_KEY_END, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_CAPS_LOCK{GLFW_KEY_CAPS_LOCK, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_SCROLL_LOCK{GLFW_KEY_SCROLL_LOCK, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_NUM_LOCK{GLFW_KEY_NUM_LOCK, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_PRINT_SCREEN{GLFW_KEY_PRINT_SCREEN, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_PAUSE{GLFW_KEY_PAUSE, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_F1{GLFW_KEY_F1, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_F2{GLFW_KEY_F2, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_F3{GLFW_KEY_F3, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_F4{GLFW_KEY_F4, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_F5{GLFW_KEY_F5, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_F6{GLFW_KEY_F6, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_F7{GLFW_KEY_F7, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_F8{GLFW_KEY_F8, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_F9{GLFW_KEY_F9, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_F10{GLFW_KEY_F10, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_F11{GLFW_KEY_F11, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_F12{GLFW_KEY_F12, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_F13{GLFW_KEY_F13, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_F14{GLFW_KEY_F14, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_F15{GLFW_KEY_F15, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_F16{GLFW_KEY_F16, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_F17{GLFW_KEY_F17, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_F18{GLFW_KEY_F18, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_F19{GLFW_KEY_F19, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_F20{GLFW_KEY_F20, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_F21{GLFW_KEY_F21, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_F22{GLFW_KEY_F22, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_F23{GLFW_KEY_F23, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_F24{GLFW_KEY_F24, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_F25{GLFW_KEY_F25, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_KP_0{GLFW_KEY_KP_0, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_KP_1{GLFW_KEY_KP_1, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_KP_2{GLFW_KEY_KP_2, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_KP_3{GLFW_KEY_KP_3, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_KP_4{GLFW_KEY_KP_4, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_KP_5{GLFW_KEY_KP_5, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_KP_6{GLFW_KEY_KP_6, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_KP_7{GLFW_KEY_KP_7, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_KP_8{GLFW_KEY_KP_8, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_KP_9{GLFW_KEY_KP_9, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_KP_DECIMAL{GLFW_KEY_KP_DECIMAL, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_KP_DIVIDE{GLFW_KEY_KP_DIVIDE, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_KP_MULTIPLY{GLFW_KEY_KP_MULTIPLY, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_KP_SUBTRACT{GLFW_KEY_KP_SUBTRACT, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_KP_ADD{GLFW_KEY_KP_ADD, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_KP_ENTER{GLFW_KEY_KP_ENTER, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_KP_EQUAL{GLFW_KEY_KP_EQUAL, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_LEFT_SHIFT{GLFW_KEY_LEFT_SHIFT, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_LEFT_CONTROL{GLFW_KEY_LEFT_CONTROL, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_LEFT_ALT{GLFW_KEY_KP_3, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_LEFT_SUPER{GLFW_KEY_LEFT_SUPER, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_RIGHT_SHIFT{GLFW_KEY_RIGHT_SHIFT, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_RIGHT_CONTROL{GLFW_KEY_RIGHT_CONTROL, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_RIGHT_ALT{GLFW_KEY_RIGHT_ALT, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_RIGHT_SUPER{GLFW_KEY_RIGHT_SUPER, CmxInputSource::KEYBOARD};
inline CmxButton CMX_KEY_MENU{GLFW_KEY_MENU, CmxInputSource::KEYBOARD};

inline CmxButton CMX_MOUSE_BUTTON_1{GLFW_MOUSE_BUTTON_1, CmxInputSource::MOUSE};
inline CmxButton CMX_MOUSE_BUTTON_2{GLFW_MOUSE_BUTTON_2, CmxInputSource::MOUSE};
inline CmxButton CMX_MOUSE_BUTTON_3{GLFW_MOUSE_BUTTON_3, CmxInputSource::MOUSE};
inline CmxButton CMX_MOUSE_BUTTON_4{GLFW_MOUSE_BUTTON_4, CmxInputSource::MOUSE};
inline CmxButton CMX_MOUSE_BUTTON_5{GLFW_MOUSE_BUTTON_5, CmxInputSource::MOUSE};
inline CmxButton CMX_MOUSE_BUTTON_6{GLFW_MOUSE_BUTTON_6, CmxInputSource::MOUSE};
inline CmxButton CMX_MOUSE_BUTTON_7{GLFW_MOUSE_BUTTON_7, CmxInputSource::MOUSE};
inline CmxButton CMX_MOUSE_BUTTON_8{GLFW_MOUSE_BUTTON_8, CmxInputSource::MOUSE};
inline CmxButton CMX_MOUSE_BUTTON_LAST{GLFW_MOUSE_BUTTON_LAST, CmxInputSource::MOUSE};
inline CmxButton CMX_MOUSE_BUTTON_LEFT{GLFW_MOUSE_BUTTON_LEFT, CmxInputSource::MOUSE};
inline CmxButton CMX_MOUSE_BUTTON_RIGHT{GLFW_MOUSE_BUTTON_RIGHT, CmxInputSource::MOUSE};
inline CmxButton CMX_MOUSE_BUTTON_MIDDLE{GLFW_MOUSE_BUTTON_MIDDLE, CmxInputSource::MOUSE};

inline CmxButton CMX_GAMEPAD_BUTTON_A{GLFW_GAMEPAD_BUTTON_A, CmxInputSource::GAMEPAD};
inline CmxButton CMX_GAMEPAD_BUTTON_B{GLFW_GAMEPAD_BUTTON_B, CmxInputSource::GAMEPAD};
inline CmxButton CMX_GAMEPAD_BUTTON_X{GLFW_GAMEPAD_BUTTON_X, CmxInputSource::GAMEPAD};
inline CmxButton CMX_GAMEPAD_BUTTON_Y{GLFW_GAMEPAD_BUTTON_Y, CmxInputSource::GAMEPAD};
inline CmxButton CMX_GAMEPAD_BUTTON_CROSS{GLFW_GAMEPAD_BUTTON_CROSS, CmxInputSource::GAMEPAD};
inline CmxButton CMX_GAMEPAD_BUTTON_CIRCLE{GLFW_GAMEPAD_BUTTON_CIRCLE, CmxInputSource::GAMEPAD};
inline CmxButton CMX_GAMEPAD_BUTTON_SQUARE{GLFW_GAMEPAD_BUTTON_SQUARE, CmxInputSource::GAMEPAD};
inline CmxButton CMX_GAMEPAD_BUTTON_TRIANGLE{GLFW_GAMEPAD_BUTTON_TRIANGLE, CmxInputSource::GAMEPAD};
inline CmxButton CMX_GAMEPAD_BUTTON_LEFT_BUMPER{GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, CmxInputSource::GAMEPAD};
inline CmxButton CMX_GAMEPAD_BUTTON_RIGHT_BUMPER{GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, CmxInputSource::GAMEPAD};
inline CmxButton CMX_GAMEPAD_BUTTON_BACK{GLFW_GAMEPAD_BUTTON_BACK, CmxInputSource::GAMEPAD};
inline CmxButton CMX_GAMEPAD_BUTTON_START{GLFW_GAMEPAD_BUTTON_START, CmxInputSource::GAMEPAD};
inline CmxButton CMX_GAMEPAD_BUTTON_GUIDE{GLFW_GAMEPAD_BUTTON_GUIDE, CmxInputSource::GAMEPAD};
inline CmxButton CMX_GAMEPAD_BUTTON_LEFT_THUMB{GLFW_GAMEPAD_BUTTON_LEFT_THUMB, CmxInputSource::GAMEPAD};
inline CmxButton CMX_GAMEPAD_BUTTON_RIGHT_THUMB{GLFW_GAMEPAD_BUTTON_RIGHT_THUMB, CmxInputSource::GAMEPAD};
inline CmxButton CMX_GAMEPAD_BUTTON_DPAD_UP{GLFW_GAMEPAD_BUTTON_DPAD_UP, CmxInputSource::GAMEPAD};
inline CmxButton CMX_GAMEPAD_BUTTON_DPAD_RIGHT{GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, CmxInputSource::GAMEPAD};
inline CmxButton CMX_GAMEPAD_BUTTON_DPAD_DOWN{GLFW_GAMEPAD_BUTTON_DPAD_DOWN, CmxInputSource::GAMEPAD};
inline CmxButton CMX_GAMEPAD_BUTTON_DPAD_LEFT{GLFW_GAMEPAD_BUTTON_DPAD_LEFT, CmxInputSource::GAMEPAD};

} // namespace cmx
