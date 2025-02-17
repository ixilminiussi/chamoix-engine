cmake_minimum_required(VERSION 3.10)
project(VulkanTest CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")

include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(.external)
add_subdirectory(cmx)
add_subdirectory(descent)

add_executable(VulkanTest descent/main.cpp) 

target_link_libraries(VulkanTest PRIVATE
    cmx
    descent
)

add_custom_target(clean_all
    COMMAND ${CMAKE_COMMAND} -E remove VulkanTest
    COMMENT "Cleaning up VulkanTest"
)
