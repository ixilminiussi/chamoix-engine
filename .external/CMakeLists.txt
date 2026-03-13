cmake_minimum_required(VERSION 3.10)

include(FetchContent)

add_subdirectory(tinyobjloader)
add_subdirectory(tinyxml2)

add_subdirectory(imgui-gradient-hotfix)

target_include_directories(imgui_gradient SYSTEM PRIVATE imgui)

# -- GLFW: always vendor from GitHub
if(NOT TARGET glfw)
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/glfw/CMakeLists.txt)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/glfw)
  else()
    message(STATUS "Fetching glfw from GitHub")
    FetchContent_Declare(
      glfw
      GIT_REPOSITORY https://github.com/glfw/glfw.git
      GIT_TAG 3.3.8
      SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/glfw
    )
    FetchContent_MakeAvailable(glfw)
  endif()
endif()

# -- spdlog: always vendor from GitHub
if(NOT TARGET spdlog)
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/spdlog/CMakeLists.txt)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/spdlog)
  else()
    message(STATUS "Fetching spdlog from GitHub")
    FetchContent_Declare(
      spdlog
      GIT_REPOSITORY https://github.com/gabime/spdlog.git
      GIT_TAG v1.12.0
      SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/spdlog
    )
    FetchContent_MakeAvailable(spdlog)
  endif()
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
