# cmx/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

add_subdirectory(editor)
add_subdirectory(systems)

add_library(cmx INTERFACE)

file(GLOB_RECURSE SYSTEMS_HEADERS LIST_DIRECTORIES true ${CMAKE_CURRENT_SOURCE_DIR}/*)
foreach(dir ${SYSTEMS_HEADERS})
    if(IS_DIRECTORY ${dir})
        target_include_directories(cmx INTERFACE $<BUILD_INTERFACE:${dir}>)
    endif()
endforeach()

target_link_libraries(cmx INTERFACE 
    editor
    systems

    spdlog glfw vulkan dl pthread X11 Xxf86vm Xrandr Xi)
