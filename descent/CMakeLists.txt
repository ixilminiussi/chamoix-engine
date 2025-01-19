# descent/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

add_library(descent
    descent_game.cpp
    wall_actor.cpp
    room_actor.cpp
    ship_actor.cpp
)

target_include_directories(descent PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(descent PUBLIC cmx tinyxml2 imgui spdlog)
