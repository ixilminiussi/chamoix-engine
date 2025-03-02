# cmx/core/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

file(GLOB CORE_SOURCES *.cpp)

target_include_directories(cmx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(actors)
add_subdirectory(components)
add_subdirectory(inputs)
add_subdirectory(physics)

target_sources(cmx PRIVATE ${CORE_SOURCES})
