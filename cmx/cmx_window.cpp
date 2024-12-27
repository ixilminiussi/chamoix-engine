#include "cmx_window.h"

// std
#include <GLFW/glfw3.h>
#include <stdexcept>

namespace cmx
{

CmxWindow::CmxWindow(int w, int h, std::string name) : _width{w}, _height{h}, _windowName{name}
{
    initWindow();
}

CmxWindow::~CmxWindow()
{
    glfwDestroyWindow(_window);
    glfwTerminate();
}

void CmxWindow::framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    CmxWindow *cmxWindow = reinterpret_cast<CmxWindow *>(glfwGetWindowUserPointer(window));
    cmxWindow->_framebufferResized = true;
    cmxWindow->_width = width;
    cmxWindow->_height = height;
}

void CmxWindow::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    _window = glfwCreateWindow(_width, _height, _windowName.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(_window, this);
    glfwSetFramebufferSizeCallback(_window, framebufferResizeCallback);
}

void CmxWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
{
    if (glfwCreateWindowSurface(instance, _window, nullptr, surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
}
} // namespace cmx
