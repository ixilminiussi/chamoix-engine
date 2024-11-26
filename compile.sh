#!/bin/bash

# List of shader filenames to compile
SHADERS=("shader.vert" "shader.frag")

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
