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

# -- Vulkan: Try to use system Vulkan first
find_package(Vulkan REQUIRED)
if(NOT Vulkan_FOUND) 
  message(STATUS "System Vulkan not found, fetching from GitHub")
  # 1A) Vulkan Headers
  FetchContent_Declare(
    VulkanHeaders
    GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
    GIT_TAG        v1.3.296    # match your desired version
    GIT_SHALLOW    TRUE
  )
  FetchContent_MakeAvailable(VulkanHeaders)

  # 1B) Vulkan Loader
  FetchContent_Declare(
    VulkanLoader
    GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Loader.git
    GIT_TAG        v1.3.296
    GIT_SHALLOW    TRUE
  )
  # tell the loader where to find the headers we just fetched:
  set(VULKAN_HEADERS_INSTALL_DIR ${vulkanheaders_SOURCE_DIR}/include CACHE PATH "" FORCE)
  FetchContent_MakeAvailable(VulkanLoader)
else()
  message(STATUS "Found system Vulkan")
endif()

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
