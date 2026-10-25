# Chamoix engine
Chamoix engine is my little vulkan/glfw/imgui game engine for 3d games with the goal of learning more about engine/graphics programming

## Compile instructions
*(tested only on linux)*

### Cloning
ensure your `.external/` directories are populated by the correct submodules, if not you may need to run

``` bash
git submodule update --init --recursive
```
or clone using
``` bash
git clone --recurse-submodules https://github.com/ixilminiussi/chamoix-engine.git
```

### Dependencies

- CMake (3.21+) and Ninja (or make)
- A C++17 compiler
- The Vulkan loader (e.g. `vulkan-loader-devel` on Fedora, `libvulkan-dev` on Debian/Ubuntu). Vulkan headers are vendored in `.external/vulkan-headers`
- `glslc` on your `PATH` (from `shaderc` / the Vulkan SDK), used to compile shaders at build time
- *Optional:* system `glfw3` and `spdlog`. If they aren't found, CMake fetches them from GitHub (GLFW then needs the X11/Wayland development headers)

### Compilation

Use `-DGAME` to pick the game directory to build (`demo`, `descent` or `petanque`, defaults to `demo`). Using one build directory per game keeps them separate.

#### For Editor (Debug):

``` bash
cmake -S . -B build/demo -GNinja -DCMAKE_BUILD_TYPE=Debug -DGAME=demo
cmake --build build/demo
./build/demo/game-bin
```

*Once in editor, use the mouse to move around, and the various editors to test out different situations*
*You can use the "game properties" window to change game inputs*
*To toggle *play* mode, press "F9"*

#### For Release:

``` bash
cmake -S . -B build/petanque-release -GNinja -DCMAKE_BUILD_TYPE=Release -DGAME=petanque
cmake --build build/petanque-release
./build/petanque-release/game-bin
```

*Release mode skips the editor and takes you straight into the game. In petanque, use WASD to move around, Mouse to look. Press Space/Left-Mouse to throw cochonet / boule, long press to throw further*

#### Presets

A `linux` preset (Debug, Ninja, default game) is also available:

``` bash
cmake --preset linux
cmake --build --preset linux
./build/linux/game-bin
```
