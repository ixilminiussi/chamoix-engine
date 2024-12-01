# .external/imgui/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

add_library(imgui
    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_widgets.cpp
    imgui_tables.cpp
    imgui_impl_vulkan.cpp
    imgui_impl_glfw.cpp
)

target_link_libraries(imgui PRIVATE glfw vulkan dl pthread X11 Xxf86vm Xrandr Xi)
