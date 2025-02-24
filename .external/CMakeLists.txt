cmake_minimum_required(VERSION 3.10)

include(FetchContent)

add_subdirectory(tinyobjloader)
add_subdirectory(tinyxml2)

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

# Cross-platform Vulkan detection
if(WIN32)
    message(STATUS "Compiling on Windows")
    
    find_package(Vulkan QUIET)

    if(NOT Vulkan_FOUND)
        set(VULKAN_SDK "$ENV{VULKAN_SDK}")
        if(VULKAN_SDK)
            set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK}/Include")
            set(Vulkan_LIBRARIES "${VULKAN_SDK}/Lib/vulkan-1.lib")
            message(STATUS "Using Vulkan from: ${VULKAN_SDK}")
        else()
            message(FATAL_ERROR "Vulkan SDK not found! Please install it and set the VULKAN_SDK environment variable.")
        endif()
    endif()

    if(NOT TARGET glfw)
        find_package(glfw3 REQUIRED)
    endif()
else()
    message(STATUS "Compiling on Linux/macOS")
    find_package(Vulkan REQUIRED)
endif()

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
