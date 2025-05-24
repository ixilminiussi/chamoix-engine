# shaders/CMakeLists.txt
set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(SHADER_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")

set(SHADER_FILES
    void.vert void.frag
    dithered_t.vert dithered_t.frag
    dithered.vert dithered.frag
    shaded.vert shaded.frag
    mesh.vert mesh.frag
    billboard.vert billboard.frag
    parallax.vert parallax.frag
    postprocess.vert
    post_passthrough.frag
    post_outline.frag
    post_ssao.frag
)

set(COMPILED_SHADERS)

foreach(SHADER ${SHADER_FILES})
    set(INPUT_FILE "${SHADER_SOURCE_DIR}/${SHADER}")
    set(OUTPUT_FILE "${SHADER_OUTPUT_DIR}/${SHADER}.spv")
    add_custom_command(
        OUTPUT ${OUTPUT_FILE}
        COMMAND glslc "${INPUT_FILE}" -o "${OUTPUT_FILE}"
        DEPENDS ${INPUT_FILE}
        COMMENT "Compiling shader ${SHADER}"
        VERBATIM
    )
    list(APPEND COMPILED_SHADERS ${OUTPUT_FILE})
endforeach()

add_custom_target(CompileShaders ALL
    DEPENDS ${COMPILED_SHADERS}
)

add_dependencies(cmx CompileShaders)

add_definitions(-DSHADER_FILES="./shaders/")

