#version 450

const vec2 OFFSETS[6] =
    vec2[](vec2(-1.0, -1.0), vec2(-1.0, 1.0), vec2(1.0, -1.0), vec2(1.0, -1.0), vec2(-1.0, 1.0), vec2(1.0, 1.0));

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec3 outRayDir;

struct DirectionalLight
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
    vec4 color;
    vec4 direction;
};

struct PointLight
{
    vec4 position;
    vec4 color;
};

layout(set = 0, binding = 0) uniform GlobalUbo
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
    vec4 ambientLight;
    vec4 cameraPosition;
    DirectionalLight sun;
    PointLight pointLights[10];
    int numPointLights;
}
ubo;

void main()
{
    gl_Position = vec4(OFFSETS[gl_VertexIndex], 0.f, 1.f);

    outUV = (OFFSETS[gl_VertexIndex] + 1.0f) / 2.0f;

    vec2 ndc = outUV * 2.0 - 1.0;
    vec4 clip = vec4(ndc, -1.0, 1.0);

    vec4 viewDir = inverse(ubo.projectionMatrix) * clip;
    viewDir /= viewDir.w;
    viewDir.w = 0.0;

    vec4 worldDir = inverse(ubo.viewMatrix) * viewDir;

    outRayDir = normalize(worldDir.xyz);
}
