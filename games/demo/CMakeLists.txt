# games/demo/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

add_library(demo
    demo_game.cpp
    cube_actor.cpp
)

target_include_directories(demo PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(demo PRIVATE cmx spdlog glfw vulkan dl pthread X11 Xxf86vm Xrandr Xi)
