#!/bin/bash

# List of shader filenames to compile
SHADERS=("void.vert" "void.frag" "dithered_t.vert" "dithered_t.frag" "dithered.vert" "dithered.frag" "shaded.vert" "shaded.frag" "mesh.vert" "mesh.frag" "billboard.vert" "billboard.frag" "parallax.vert" "parallax.frag" "hud.vert" "hud.frag")

# Source and output directories (update these paths as needed)
SOURCE_DIR="../shaders"   # Folder where your shaders are located
OUTPUT_DIR="./shaders"    # Folder where compiled SPIR-V files will be placed

# Ensure the output directory exists
mkdir -p "$OUTPUT_DIR"

# Function to compile a shader
compile_shader() {
    local shader_file="$1"
    local input_path="$SOURCE_DIR/$shader_file"
    local output_path="$OUTPUT_DIR/$shader_file.spv"

    # Check if the shader file exists
    if [ -f "$input_path" ]; then
        echo "Compiling $shader_file..."
        glslc "$input_path" -o "$output_path"
        if [ $? -eq 0 ]; then
            echo "Compiled $shader_file to $output_path"
        else
            echo "Error compiling $shader_file"
        fi
    else
        echo "Shader file $shader_file not found in $SOURCE_DIR"
    fi
}

# Compile each shader in the list
for shader in "${SHADERS[@]}"; do
    compile_shader "$shader"
done

echo "Shader compilation complete."

# Copying cmx assets to build directory
SOURCE_DIR="../assets"
OUTPUT_DIR="assets/cmx"

mkdir -p "$OUTPUT_DIR"

cp -r $SOURCE_DIR/* $OUTPUT_DIR/
echo "Cmx assets copied"

# Copying game assets to build directory
SOURCE_DIR="../$1"
OUTPUT_DIR="."

# Ensure the ouput directory exists
mkdir -p "$OUTPUT_DIR"

# Copy over the files
cp -r $SOURCE_DIR/assets $OUTPUT_DIR/
echo "Assets copied."

cp -r $SOURCE_DIR/scenes $OUTPUT_DIR/
echo "Scenes copied."

cp -r $SOURCE_DIR/properties $OUTPUT_DIR/
echo "Properties copied."


# Copying editor to the build directory
SOURCE_DIR="../editor"
OUTPUT_DIR="./editor"

# Ensure the ouput directory exists
mkdir -p "$OUTPUT_DIR"

# Copy over the files
cp -r $SOURCE_DIR/* $OUTPUT_DIR/
echo "Editor assets copied."
