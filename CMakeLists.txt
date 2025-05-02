cmake_minimum_required(VERSION 3.10)
project(VulkanTest CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
else()
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
endif()

include_directories(${CMAKE_SOURCE_DIR})

add_compile_definitions(_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING)

add_subdirectory(.external)
add_subdirectory(cmx)
add_subdirectory(demo)

add_executable(VulkanTest demo/main.cpp) 

target_link_libraries(VulkanTest PRIVATE
    cmx
    demo
)

set(CMX_FILES "${CMAKE_SOURCE_DIR}/assets")

set(CMX_EDITOR_FILES "${CMAKE_SOURCE_DIR}/editor")
add_definitions(-DCMX_EDITOR_FILES="${CMX_FILES}")

set(GAME_FILES "${CMAKE_SOURCE_DIR}/demo")
add_definitions(-DGAME_FILES="${GAME_FILES}")

file(GLOB CMX_ASSETS "${CMX_FILES}/*")
file(COPY ${CMX_ASSETS} DESTINATION "${GAME_FILES}/cmx")

add_custom_target(clean_all
    COMMAND ${CMAKE_COMMAND} -E remove VulkanTest
    COMMENT "Cleaning up VulkanTest"
)
