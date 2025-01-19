cmake_minimum_required(VERSION 3.10)

# Add existing subdirectories
add_subdirectory(tinyobjloader)
add_subdirectory(imgui)
add_subdirectory(tinyxml2)

# Define the iconfonts target directly here
add_library(iconfonts INTERFACE)

target_include_directories(iconfonts INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/iconfonts>
)

