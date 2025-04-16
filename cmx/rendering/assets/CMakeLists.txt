# cmx/rendering/assets/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

target_include_directories(cmx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(materials)
add_subdirectory(postprocesses)

file(GLOB RENDERING_ASSETS_SOURCES *.cpp)

target_sources(cmx PRIVATE ${RENDERING_ASSETS_SOURCES})
