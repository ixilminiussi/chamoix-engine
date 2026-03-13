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

# -- Vulkan detection (done here so Vulkan::Vulkan is available to all subdirectories)
if(WIN32)
    find_package(Vulkan QUIET)
    if(NOT Vulkan_FOUND)
        set(VULKAN_SDK "$ENV{VULKAN_SDK}")
        if(NOT VULKAN_SDK)
            set(VULKAN_SDK "$ENV{VK_SDK_PATH}")
        endif()
        if(VULKAN_SDK)
            set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK}/Include")
            set(Vulkan_LIBRARIES "${VULKAN_SDK}/Lib/vulkan-1.lib")
            set(Vulkan_FOUND TRUE)
            message(STATUS "Using Vulkan from VULKAN_SDK: ${VULKAN_SDK}")
        else()
            message(FATAL_ERROR "Vulkan SDK not found! Install it and set VULKAN_SDK.")
        endif()
    endif()
else()
    find_package(Vulkan REQUIRED)
endif()

if(NOT TARGET Vulkan::Vulkan)
    add_library(Vulkan::Vulkan UNKNOWN IMPORTED GLOBAL)
    set_target_properties(Vulkan::Vulkan PROPERTIES
        IMPORTED_LOCATION "${Vulkan_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${Vulkan_INCLUDE_DIRS}"
    )
endif()

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
