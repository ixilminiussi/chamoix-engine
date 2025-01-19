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

``` bash
mkdir build
cd build
../compile.sh descent # copies the game files into build, compiles shaders
cmake -GNinja -DCMAKE_BUILD_TYPE=Debug .. # build can be rather slow, ninja is recommended for faster compilation. make also works
ninja
./VulkanTest
```


