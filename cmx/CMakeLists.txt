# cmx/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

file(GLOB SOURCES *.cpp)

add_library(cmx ${SOURCES})

target_link_libraries(cmx PRIVATE tinyxml2 imgui tinyobjloader spdlog iconfonts glfw vulkan dl pthread X11 Xxf86vm Xrandr Xi)

