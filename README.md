# Chamoix engine
Chamoix engine is my little vulkan/glfw/imgui game engine for 3d games with the goal of learning more about engine/graphics programming

## Compile instructions
*(tested only on linux)*
- make sure your .external/ folder includes iconfonts, imgui, stb, tinyobjloader and tinyxml2
- install x11, glfw, vulkan

``` bash
mkdir build
cd build
../compile.sh descent # copies the game files into build, compiles shaders
cmake -GNinja -DCMAKE_BUILD_TYPE=Debug .. # build can be rather slow, ninja is recommended for faster compilation. make also works
ninja
./VulkanTest
```


