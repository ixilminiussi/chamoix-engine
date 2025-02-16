# petanque/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

add_library(petanque
    petanque_game.cpp
    static_body_actor.cpp
    boule_actor.cpp
    dynamic_body_actor.cpp
    rigid_body_actor.cpp
    first_person_actor.cpp
)

target_include_directories(petanque PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(petanque PUBLIC cmx tinyxml2 imgui spdlog)
