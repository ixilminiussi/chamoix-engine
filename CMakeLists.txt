cmake_minimum_required(VERSION 3.10)
project(VulkanTest CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")

include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(.external)
add_subdirectory(cmx)
add_subdirectory(demo)

# Create the executable by linking all modules
add_executable(VulkanTest demo/main.cpp)  # Assuming main.cpp is at the root level

# Link necessary libraries and internal modules
target_link_libraries(VulkanTest PRIVATE
    cmx
    demo
)

# Define custom targets for 'test' and 'clean'
add_custom_target(test
    COMMAND ${CMAKE_COMMAND} -E env bash ${CMAKE_SOURCE_DIR}/compile.sh demo
    COMMAND VulkanTest
    DEPENDS VulkanTest
    COMMENT "Running VulkanTest"
)

add_custom_target(clean_all
    COMMAND ${CMAKE_COMMAND} -E remove VulkanTest
    COMMENT "Cleaning up VulkanTest"
)
