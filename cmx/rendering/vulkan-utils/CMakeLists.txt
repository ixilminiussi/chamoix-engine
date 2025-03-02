# cmx/rendering/vulkan-utils/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

target_include_directories(cmx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB RENDERING_VULKAN_UTILS_SOURCES *.cpp)

target_sources(cmx PRIVATE ${RENDERING_VULKAN_UTILS_SOURCES})
