cmake_minimum_required(VERSION 3.10)
project(VulkanTest CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED GAME)
    set(GAME "demo" CACHE STRING "Choose game module to build (e.g. demo, descent)")
endif()

# Set game directory
set(GAME_DIR "${CMAKE_SOURCE_DIR}/${GAME}/")
add_definitions(-DGAME_FILES="${GAME_DIR}")

# Compiler flags
if(WIN32)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
endif()

include_directories(${CMAKE_SOURCE_DIR})

add_compile_definitions(_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING)

# Asset and shader paths
set(CMX_FILES "${CMAKE_SOURCE_DIR}/assets/")
set(CMX_EDITOR_FILES "${CMAKE_SOURCE_DIR}/editor/")
add_definitions(-DEDITOR_FILES="${CMX_EDITOR_FILES}")

set(SHADER_FILES "./shaders/")
add_definitions(-DSHADER_FILES="${SHADER_FILES}")

# Add subdirectories
add_subdirectory(.external)
add_subdirectory(cmx)
add_subdirectory(${GAME})
add_subdirectory(shaders)

# Executable
add_executable(VulkanTest ${GAME}/main.cpp)

target_link_libraries(VulkanTest PRIVATE
    cmx
    ${GAME}
)

# Copy assets
file(GLOB CMX_ASSETS "${CMX_FILES}/*")
file(COPY ${CMX_ASSETS} DESTINATION "${GAME_DIR}/.cmx_assets")
file(MAKE_DIRECTORY "${GAME_DIR}/.editor")

message(STATUS "Copied cmx assets into game directory")

# Clean target
add_custom_target(clean_all
    COMMAND ${CMAKE_COMMAND} -E remove VulkanTest
    COMMENT "Cleaning up VulkanTest"
)
