#pragma once

// lib
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// std
#include <string>

namespace cmx
{

class CmxWindow
{
  public:
    CmxWindow(int w, int h, std::string name);
    ~CmxWindow();

    CmxWindow(const CmxWindow &) = delete;
    CmxWindow &operator=(const CmxWindow &) = delete;

    bool shouldClose() const
    {
        return glfwWindowShouldClose(_window);
    }
    VkExtent2D getExtent() const
    {
        return {static_cast<uint32_t>(_width), static_cast<uint32_t>(_height)};
    }
    bool wasWindowResized() const
    {
        return _framebufferResized;
    }
    void resetWindowResizedFlag()
    {
        _framebufferResized = false;
    }
    GLFWwindow *getGLFWwindow() const
    {
        return _window;
    }

    void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

  private:
    static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
    void initWindow();

    int _width;
    int _height;
    bool _framebufferResized = false;

    std::string _windowName;
    GLFWwindow *_window;
};
} // namespace cmx
