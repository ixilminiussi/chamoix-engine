# descent/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

add_library(descent
    descent_game.cpp
    bullet_actor.cpp
    descent_game.cpp
    enemy_ship_actor.cpp
    ship_actor.cpp
    ship_camera_component.cpp
    wall_actor.cpp
)

target_link_libraries(descent PUBLIC cmx tinyxml2 imgui spdlog)
