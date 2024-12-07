# external/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

add_subdirectory(tinyobjloader)
add_subdirectory(imgui)
add_subdirectory(tinyxml2)

add_library(external INTERFACE)

file(GLOB_RECURSE SYSTEMS_HEADERS LIST_DIRECTORIES true ${CMAKE_CURRENT_SOURCE_DIR}/*)
foreach(dir ${SYSTEMS_HEADERS})
    if(IS_DIRECTORY ${dir})
        target_include_directories(cmx INTERFACE $<BUILD_INTERFACE:${dir}>)
    endif()
endforeach()

target_link_libraries(external INTERFACE tinyobjloader imgui tinyxml2)
