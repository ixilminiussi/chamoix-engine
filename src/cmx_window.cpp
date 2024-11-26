#include "cmx_window.h"

// std
#include <GLFW/glfw3.h>
#include <stdexcept>

namespace cmx
{

CmxWindow::CmxWindow(int w, int h, std::string name) : width{w}, height{h}, windowName{name}
{
    initWindow();
}

CmxWindow::~CmxWindow()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void CmxWindow::framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    CmxWindow *cmxWindow = reinterpret_cast<CmxWindow *>(glfwGetWindowUserPointer(window));
    cmxWindow->framebufferResized = true;
    cmxWindow->width = width;
    cmxWindow->height = height;
}

void CmxWindow::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void CmxWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
{
    if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
}
} // namespace cmx
