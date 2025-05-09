# ![Surface Stable Dithering Explained](https://www.notion.so/Surface-stable-fractal-dithering-1dbe6f2472698047ad97d559e5174fc7?pvs=4)

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

### Compilation

#### For Editor:

``` bash
mkdir build
cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Debug .. -DGAME=demo # use -DGAME to specify the game directory you intend to build
ninja # or make if using make
./VulkanTest
```

*Once in editor, use the mouse to move around, and the various editors to test out different situations*
*You can use the "game properties" window to change game inputs*
*To toggle *play* mode, press "F9"*

#### For Release:

``` bash
mkdir build
cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Release .. -DGAME=demo
ninja
./VulkanTest
```

*Release mode essentially takes you straight to the petanque game. Use WASD to move around, Mouse to look. Press Space/Left-Mouse to throw cochonet / boule, long press to throw further* 

