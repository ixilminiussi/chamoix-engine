# cmx/systems/game/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

add_library(game
    entities/cmx_actor.cpp
    entities/cmx_component.cpp
    entities/cmx_camera_component.cpp
    entities/cmx_mesh_actor.cpp
    entities/cmx_mesh_component.cpp
    entities/cmx_transform.cpp
    cmx_game.cpp
    cmx_scene.cpp
)

target_link_libraries(game PRIVATE cmx demo spdlog)
