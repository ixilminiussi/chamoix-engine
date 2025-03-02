# cmx/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(cmx)

add_library(cmx)

target_include_directories(cmx PUBLIC ${CMAKE_SOURCE_DIR}/.external/stb)

add_subdirectory(editor)
add_subdirectory(core)
add_subdirectory(misc)
add_subdirectory(rendering)

target_link_libraries(cmx PRIVATE tinyxml2 imgui imguizmo imgui_gradient tinyobjloader spdlog iconfonts glfw vulkan dl pthread X11 Xxf86vm Xrandr Xi)
