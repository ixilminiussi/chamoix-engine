#include "cmx_window.h"

// lib
#include <GLFW/glfw3.h>

// std
#include <stdexcept>

namespace cmx
{

Window::Window(int w, int h, std::string name) : _width{w}, _height{h}, _windowName{name}
{
    initWindow();
}

Window::~Window()
{
    glfwDestroyWindow(_glfwWindow);
    glfwTerminate();
}

void Window::framebufferResizeCallback(GLFWwindow *glfwWindow, int width, int height)
{
    Window *window = reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfwWindow));
    window->_framebufferResized = true;
    window->_width = width;
    window->_height = height;
}

void Window::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    _glfwWindow = glfwCreateWindow(_width, _height, _windowName.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(_glfwWindow, this);
    glfwSetFramebufferSizeCallback(_glfwWindow, framebufferResizeCallback);
}

void Window::createWindowSurface(vk::Instance instance, vk::SurfaceKHR *surface)
{
    if (glfwCreateWindowSurface(static_cast<VkInstance>(instance), _glfwWindow, nullptr,
                                reinterpret_cast<VkSurfaceKHR *>(surface)) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
}
} // namespace cmx
