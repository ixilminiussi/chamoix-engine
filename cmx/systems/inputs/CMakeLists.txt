# cmx/systems/inputs/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

add_library(inputs
    cmx_input_action.cpp
    cmx_input_manager.cpp
)

target_link_libraries(inputs PRIVATE cmx spdlog glfw vulkan dl pthread X11 Xxf86vm Xrandr Xi)
