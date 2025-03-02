# cmx/rendering/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

file(GLOB RENDERING_SOURCES *.cpp)

target_include_directories(cmx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(assets)
add_subdirectory(vulkan-utils)

target_sources(cmx PRIVATE ${RENDERING_SOURCES})
