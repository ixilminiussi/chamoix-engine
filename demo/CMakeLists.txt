# demo/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

add_library(demo
    demo_game.cpp
    first_person_actor.cpp
    rigid_body_actor.cpp
    static_body_actor.cpp
    dynamic_body_actor.cpp
    rotating_actor.cpp
)

target_link_libraries(demo PUBLIC cmx tinyxml2 imgui spdlog)
