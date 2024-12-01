# cmx/editor/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

add_library(editor
    cmx_viewport_actor.cpp
    cmx_viewport_ui_component.cpp
)

target_link_libraries(editor PRIVATE cmx spdlog glfw vulkan dl pthread X11 Xxf86vm Xrandr Xi)
