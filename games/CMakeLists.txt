# games/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

add_subdirectory(demo)

add_library(games INTERFACE)

target_link_libraries(games INTERFACE 
    demo

    spdlog glfw vulkan dl pthread X11 Xxf86vm Xrandr Xi)
