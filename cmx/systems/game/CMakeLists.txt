# cmx/systems/game/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

add_library(game
    cmx_actor.cpp
    cmx_game.cpp
    cmx_world.cpp
    cmx_transform.cpp
)

target_link_libraries(game PRIVATE cmx spdlog glfw vulkan dl pthread X11 Xxf86vm Xrandr Xi)
