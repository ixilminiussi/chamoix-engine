cmake_minimum_required(VERSION 3.10)
project(VulkanTest CXX)

# Set the C++ standard and compiler flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")


# Add the include directory
include_directories(${CMAKE_SOURCE_DIR}/include)

# Add the source files from the 'src' directory
file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)

# Add the executable target
add_executable(VulkanTest ${SOURCES})

# Link the necessary libraries
target_link_libraries(VulkanTest PRIVATE spdlog glfw vulkan dl pthread X11 Xxf86vm Xrandr Xi)

# Define custom targets for 'test' and 'clean'
add_custom_target(test
    COMMAND ${CMAKE_COMMAND} -E env bash ${CMAKE_SOURCE_DIR}/compile.sh
    COMMAND VulkanTest
    DEPENDS VulkanTest
    COMMENT "Running VulkanTest"
)

add_custom_target(clean_all
    COMMAND ${CMAKE_COMMAND} -E remove VulkanTest
    COMMENT "Cleaning up VulkanTest"
)
