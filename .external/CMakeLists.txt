cmake_minimum_required(VERSION 3.10)

add_subdirectory(tinyobjloader)
add_subdirectory(tinyxml2)

add_library(iconfonts INTERFACE)

target_include_directories(iconfonts INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/iconfonts>
)

add_library(imgui STATIC
  imgui/imgui.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_widgets.cpp
  imgui/imgui_tables.cpp
  imgui/backends/imgui_impl_glfw.cpp  
  imgui/backends/imgui_impl_vulkan.cpp  
)

target_include_directories(imgui PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/imgui>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends>  
)

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)

target_link_libraries(imgui PUBLIC glfw Vulkan::Vulkan)
