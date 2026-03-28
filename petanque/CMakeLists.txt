# petanque/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

add_library(petanque
    petanque_game.cpp
    boule_actor.cpp
    first_person_actor.cpp
    rigid_body_actor.cpp
    static_body_actor.cpp
    dynamic_body_actor.cpp
)

target_link_libraries(petanque PUBLIC cmx tinyxml2 imgui spdlog)
