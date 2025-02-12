#version 450

const vec2 OFFSETS[6] =
    vec2[](vec2(-1.0, -1.0), vec2(-1.0, 1.0), vec2(1.0, -1.0), vec2(1.0, -1.0), vec2(-1.0, 1.0), vec2(1.0, 1.0));

layout(location = 0) out vec2 fragUV;

void main()
{
    fragUV = (OFFSETS[gl_VertexIndex] + 1.0f) / 2.0f;

    gl_Position = vec4(OFFSETS[gl_VertexIndex], 0.f, 1.f);
}
