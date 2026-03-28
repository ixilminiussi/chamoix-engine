# .external/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

include(FetchContent)

add_subdirectory(tinyobjloader)
add_subdirectory(tinyxml2)

set(SPIRV_REFLECT_STATIC_LIB ON CACHE BOOL "" FORCE)
add_subdirectory(spirv-reflect)
add_subdirectory(imgui-gradient-hotfix)

target_include_directories(imgui_gradient SYSTEM PRIVATE imgui)

add_compile_definitions(VULKAN_HPP_TYPESAFE_CONVERSION)

## -- boost: Try to use system boost first
cmake_policy(SET CMP0167 OLD)
cmake_policy(SET CMP0135 NEW)
cmake_policy(SET CMP0169 NEW)
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS  TRUE)

# -- Vulkan: vendored headers + system loader
find_library(Vulkan_LIBRARY NAMES vulkan
  HINTS /usr/lib64 /usr/lib/x86_64-linux-gnu
  REQUIRED
)
add_library(Vulkan::Vulkan UNKNOWN IMPORTED)
set_target_properties(Vulkan::Vulkan PROPERTIES
  IMPORTED_LOCATION "${Vulkan_LIBRARY}"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/vulkan-headers"
)
message(STATUS "Vulkan library: ${Vulkan_LIBRARY}")

# -- GLFW: Try to use system glfw3 first
find_package(glfw3 QUIET)
if(NOT glfw3_FOUND)
  message(STATUS "System glfw3 not found, fetching from GitHub")
  FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8 
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/glfw
  )
  FetchContent_MakeAvailable(glfw)
else()
  message(STATUS "Found system glfw3")
endif()

# -- spdlog: Try to use system spdlog first
find_package(spdlog QUIET)
if(NOT spdlog_FOUND)
  message(STATUS "System spdlog not found, fetching from GitHub")
  FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.12.0
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/spdlog
  )
  FetchContent_MakeAvailable(spdlog)
else()
  message(STATUS "Found system spdlog")
endif()

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
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/imgui-filebrowser>
)

target_link_libraries(imgui PUBLIC glfw Vulkan::Vulkan)

add_library(imguizmo STATIC
    imguizmo/GraphEditor.cpp
    imguizmo/ImCurveEdit.cpp
    imguizmo/ImGradient.cpp
    imguizmo/ImGuizmo.cpp
    imguizmo/ImSequencer.cpp
)

target_include_directories(imguizmo PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/imguizmo>
)

target_link_libraries(imguizmo PUBLIC imgui)
