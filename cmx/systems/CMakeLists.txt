# cmx/systems/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

add_subdirectory(game)
add_subdirectory(inputs)
add_subdirectory(rendering)

add_library(systems INTERFACE)

target_link_libraries(systems INTERFACE
    game
    inputs
    rendering

    cmx spdlog glfw vulkan dl pthread X11 Xxf86vm Xrandr Xi
)
