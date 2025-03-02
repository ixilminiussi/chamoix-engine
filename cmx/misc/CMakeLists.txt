# cmx/misc/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

target_include_directories(cmx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB MISC_SOURCES *.cpp)

target_sources(cmx PRIVATE ${MISC_SOURCES})
