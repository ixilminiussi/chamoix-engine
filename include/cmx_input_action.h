#pragma once

// lib
#include <functional>
#include <glm/fwd.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// std
#include <initializer_list>
#include <string>
#include <vector>

namespace cmx
{

enum CmxInputType
{
    GAMEPAD,
    GAMEPAD_AXIS,
    MOUSE,
    MOUSE_AXIS,
    KEYBOARD
};

enum CmxInputCategory
{
    BUTTON,
    AXIS,
};

struct CmxInput
{
    int code;
    CmxInputType type;
    int status{GLFW_RELEASE};
};

class CmxInputAction
{
  public:
    CmxInputAction(const CmxInputCategory, std::initializer_list<CmxInput>);

    void poll(const class CmxWindow &);
    void bind(std::function<void(bool)>);
    void bind(std::function<void(glm::vec2)>);

    const CmxInputCategory inputCategory;

  private:
    void validateInputs();

    std::vector<CmxInput> inputs;

    std::vector<std::function<void(bool)>> buttonFunctions;
    std::vector<std::function<void(glm::vec2)>> axisFunctions;
};

inline CmxInput CMX_KEY_SPACE{GLFW_KEY_SPACE, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_APOSTROPHE{GLFW_KEY_APOSTROPHE, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_COMMA{GLFW_KEY_COMMA, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_MINUS{GLFW_KEY_MINUS, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_PERIOD{GLFW_KEY_PERIOD, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_SLASH{GLFW_KEY_SLASH, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_0{GLFW_KEY_0, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_1{GLFW_KEY_1, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_2{GLFW_KEY_2, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_3{GLFW_KEY_3, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_4{GLFW_KEY_4, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_5{GLFW_KEY_5, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_6{GLFW_KEY_6, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_7{GLFW_KEY_7, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_8{GLFW_KEY_8, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_9{GLFW_KEY_9, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_SEMICOLON{GLFW_KEY_SEMICOLON, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_EQUAL{GLFW_KEY_EQUAL, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_A{GLFW_KEY_A, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_B{GLFW_KEY_B, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_C{GLFW_KEY_C, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_D{GLFW_KEY_D, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_E{GLFW_KEY_E, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_F{GLFW_KEY_F, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_H{GLFW_KEY_H, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_I{GLFW_KEY_I, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_J{GLFW_KEY_J, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_K{GLFW_KEY_K, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_L{GLFW_KEY_L, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_M{GLFW_KEY_M, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_N{GLFW_KEY_N, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_O{GLFW_KEY_O, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_P{GLFW_KEY_P, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_Q{GLFW_KEY_Q, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_R{GLFW_KEY_R, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_S{GLFW_KEY_S, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_T{GLFW_KEY_T, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_U{GLFW_KEY_U, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_V{GLFW_KEY_V, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_W{GLFW_KEY_W, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_X{GLFW_KEY_X, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_Y{GLFW_KEY_Y, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_Z{GLFW_KEY_Z, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_LEFT_BRACKET{GLFW_KEY_LEFT_BRACKET, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_BACKSLASH{GLFW_KEY_BACKSLASH, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_RIGHT_BRACKET{GLFW_KEY_RIGHT_BRACKET, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_GRAVE_ACCENT{GLFW_KEY_GRAVE_ACCENT, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_WORLD_1{GLFW_KEY_WORLD_1, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_WORLD_2{GLFW_KEY_WORLD_2, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_ESCAPE{GLFW_KEY_ESCAPE, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_ENTER{GLFW_KEY_ENTER, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_TAB{GLFW_KEY_TAB, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_BACKSPACE{GLFW_KEY_BACKSPACE, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_INSERT{GLFW_KEY_INSERT, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_DELETE{GLFW_KEY_DELETE, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_RIGHT{GLFW_KEY_RIGHT, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_LEFT{GLFW_KEY_LEFT, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_DOWN{GLFW_KEY_DOWN, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_UP{GLFW_KEY_UP, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_PAGE_UP{GLFW_KEY_PAGE_UP, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_PAGE_DOWN{GLFW_KEY_PAGE_DOWN, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_HOME{GLFW_KEY_HOME, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_END{GLFW_KEY_END, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_CAPS_LOCK{GLFW_KEY_CAPS_LOCK, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_SCROLL_LOCK{GLFW_KEY_SCROLL_LOCK, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_NUM_LOCK{GLFW_KEY_NUM_LOCK, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_PRINT_SCREEN{GLFW_KEY_PRINT_SCREEN, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_PAUSE{GLFW_KEY_PAUSE, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_F1{GLFW_KEY_F1, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_F2{GLFW_KEY_F2, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_F3{GLFW_KEY_F3, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_F4{GLFW_KEY_F4, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_F5{GLFW_KEY_F5, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_F6{GLFW_KEY_F6, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_F7{GLFW_KEY_F7, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_F8{GLFW_KEY_F8, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_F9{GLFW_KEY_F9, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_F10{GLFW_KEY_F10, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_F11{GLFW_KEY_F11, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_F12{GLFW_KEY_F12, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_F13{GLFW_KEY_F13, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_F14{GLFW_KEY_F14, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_F15{GLFW_KEY_F15, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_F16{GLFW_KEY_F16, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_F17{GLFW_KEY_F17, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_F18{GLFW_KEY_F18, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_F19{GLFW_KEY_F19, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_F20{GLFW_KEY_F20, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_F21{GLFW_KEY_F21, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_F22{GLFW_KEY_F22, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_F23{GLFW_KEY_F23, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_F24{GLFW_KEY_F24, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_F25{GLFW_KEY_F25, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_KP_0{GLFW_KEY_KP_0, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_KP_1{GLFW_KEY_KP_1, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_KP_2{GLFW_KEY_KP_2, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_KP_3{GLFW_KEY_KP_3, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_KP_4{GLFW_KEY_KP_4, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_KP_5{GLFW_KEY_KP_5, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_KP_6{GLFW_KEY_KP_6, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_KP_7{GLFW_KEY_KP_7, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_KP_8{GLFW_KEY_KP_8, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_KP_9{GLFW_KEY_KP_9, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_KP_DECIMAL{GLFW_KEY_KP_DECIMAL, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_KP_DIVIDE{GLFW_KEY_KP_DIVIDE, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_KP_MULTIPLY{GLFW_KEY_KP_MULTIPLY, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_KP_SUBTRACT{GLFW_KEY_KP_SUBTRACT, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_KP_ADD{GLFW_KEY_KP_ADD, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_KP_ENTER{GLFW_KEY_KP_ENTER, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_KP_EQUAL{GLFW_KEY_KP_EQUAL, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_LEFT_SHIFT{GLFW_KEY_LEFT_SHIFT, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_LEFT_CONTROL{GLFW_KEY_LEFT_CONTROL, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_LEFT_ALT{GLFW_KEY_KP_3, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_LEFT_SUPER{GLFW_KEY_LEFT_SUPER, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_RIGHT_SHIFT{GLFW_KEY_RIGHT_SHIFT, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_RIGHT_CONTROL{GLFW_KEY_RIGHT_CONTROL, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_RIGHT_ALT{GLFW_KEY_RIGHT_ALT, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_RIGHT_SUPER{GLFW_KEY_RIGHT_SUPER, CmxInputType::KEYBOARD};
inline CmxInput CMX_KEY_MENU{GLFW_KEY_MENU, CmxInputType::KEYBOARD};

inline CmxInput CMX_MOUSE_BUTTON_1{GLFW_MOUSE_BUTTON_1, CmxInputType::MOUSE};
inline CmxInput CMX_MOUSE_BUTTON_2{GLFW_MOUSE_BUTTON_2, CmxInputType::MOUSE};
inline CmxInput CMX_MOUSE_BUTTON_3{GLFW_MOUSE_BUTTON_3, CmxInputType::MOUSE};
inline CmxInput CMX_MOUSE_BUTTON_4{GLFW_MOUSE_BUTTON_4, CmxInputType::MOUSE};
inline CmxInput CMX_MOUSE_BUTTON_5{GLFW_MOUSE_BUTTON_5, CmxInputType::MOUSE};
inline CmxInput CMX_MOUSE_BUTTON_6{GLFW_MOUSE_BUTTON_6, CmxInputType::MOUSE};
inline CmxInput CMX_MOUSE_BUTTON_7{GLFW_MOUSE_BUTTON_7, CmxInputType::MOUSE};
inline CmxInput CMX_MOUSE_BUTTON_8{GLFW_MOUSE_BUTTON_8, CmxInputType::MOUSE};
inline CmxInput CMX_MOUSE_BUTTON_LAST{GLFW_MOUSE_BUTTON_LAST, CmxInputType::MOUSE};
inline CmxInput CMX_MOUSE_BUTTON_LEFT{GLFW_MOUSE_BUTTON_LEFT, CmxInputType::MOUSE};
inline CmxInput CMX_MOUSE_BUTTON_RIGHT{GLFW_MOUSE_BUTTON_RIGHT, CmxInputType::MOUSE};
inline CmxInput CMX_MOUSE_BUTTON_MIDDLE{GLFW_MOUSE_BUTTON_MIDDLE, CmxInputType::MOUSE};

inline CmxInput CMX_GAMEPAD_BUTTON_A{GLFW_GAMEPAD_BUTTON_A, CmxInputType::GAMEPAD};
inline CmxInput CMX_GAMEPAD_BUTTON_B{GLFW_GAMEPAD_BUTTON_B, CmxInputType::GAMEPAD};
inline CmxInput CMX_GAMEPAD_BUTTON_X{GLFW_GAMEPAD_BUTTON_X, CmxInputType::GAMEPAD};
inline CmxInput CMX_GAMEPAD_BUTTON_Y{GLFW_GAMEPAD_BUTTON_Y, CmxInputType::GAMEPAD};
inline CmxInput CMX_GAMEPAD_BUTTON_CROSS{GLFW_GAMEPAD_BUTTON_CROSS, CmxInputType::GAMEPAD};
inline CmxInput CMX_GAMEPAD_BUTTON_CIRCLE{GLFW_GAMEPAD_BUTTON_CIRCLE, CmxInputType::GAMEPAD};
inline CmxInput CMX_GAMEPAD_BUTTON_SQUARE{GLFW_GAMEPAD_BUTTON_SQUARE, CmxInputType::GAMEPAD};
inline CmxInput CMX_GAMEPAD_BUTTON_TRIANGLE{GLFW_GAMEPAD_BUTTON_TRIANGLE, CmxInputType::GAMEPAD};
inline CmxInput CMX_GAMEPAD_BUTTON_LEFT_BUMPER{GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, CmxInputType::GAMEPAD};
inline CmxInput CMX_GAMEPAD_BUTTON_RIGHT_BUMPER{GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, CmxInputType::GAMEPAD};
inline CmxInput CMX_GAMEPAD_BUTTON_BACK{GLFW_GAMEPAD_BUTTON_BACK, CmxInputType::GAMEPAD};
inline CmxInput CMX_GAMEPAD_BUTTON_START{GLFW_GAMEPAD_BUTTON_START, CmxInputType::GAMEPAD};
inline CmxInput CMX_GAMEPAD_BUTTON_GUIDE{GLFW_GAMEPAD_BUTTON_GUIDE, CmxInputType::GAMEPAD};
inline CmxInput CMX_GAMEPAD_BUTTON_LEFT_THUMB{GLFW_GAMEPAD_BUTTON_LEFT_THUMB, CmxInputType::GAMEPAD};
inline CmxInput CMX_GAMEPAD_BUTTON_RIGHT_THUMB{GLFW_GAMEPAD_BUTTON_RIGHT_THUMB, CmxInputType::GAMEPAD};
inline CmxInput CMX_GAMEPAD_BUTTON_DPAD_UP{GLFW_GAMEPAD_BUTTON_DPAD_UP, CmxInputType::GAMEPAD};
inline CmxInput CMX_GAMEPAD_BUTTON_DPAD_RIGHT{GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, CmxInputType::GAMEPAD};
inline CmxInput CMX_GAMEPAD_BUTTON_DPAD_DOWN{GLFW_GAMEPAD_BUTTON_DPAD_DOWN, CmxInputType::GAMEPAD};
inline CmxInput CMX_GAMEPAD_BUTTON_DPAD_LEFT{GLFW_GAMEPAD_BUTTON_DPAD_LEFT, CmxInputType::GAMEPAD};

} // namespace cmx
