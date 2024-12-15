# cmx/systems/assets/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

add_library(assets
    cmx_assets_manager.cpp
)

target_link_libraries(assets PRIVATE cmx spdlog glfw vulkan)
