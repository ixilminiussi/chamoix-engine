# cmx/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

file(GLOB SOURCES *.cpp)

add_library(cmx ${SOURCES})

target_include_directories(cmx PRIVATE ${CMAKE_SOURCE_DIR}/.external/stb)

target_link_libraries(cmx PRIVATE tinyxml2 imgui imguizmo imgui_gradient tinyobjloader spdlog iconfonts glfw Vulkan::Vulkan)

if(UNIX AND NOT APPLE)
  target_link_libraries(cmx PRIVATE dl pthread X11 Xxf86vm Xrandr Xi)
endif()
