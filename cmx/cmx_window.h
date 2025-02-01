#ifndef CMX_WINDOW
#define CMX_WINDOW

// lib
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

// std
#include <string>

namespace cmx
{

class Window
{
  public:
    Window(int w, int h, std::string name);
    ~Window();

    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;

    bool shouldClose() const
    {
        return glfwWindowShouldClose(_glfwWindow);
    }
    vk::Extent2D getExtent() const
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
        return _glfwWindow;
    }

    void createWindowSurface(vk::Instance instance, vk::SurfaceKHR *surface);

  private:
    static void framebufferResizeCallback(GLFWwindow *, int width, int height);
    void initWindow();

    int _width;
    int _height;
    bool _framebufferResized = false;

    std::string _windowName;
    GLFWwindow *_glfwWindow;
};

} // namespace cmx

#endif
