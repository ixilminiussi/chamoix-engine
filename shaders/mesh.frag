#version 450

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec3 inPositionWorld;
layout(location = 2) in vec3 inNormalWorld;

layout(location = 0) out vec4 outColor;

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
    PointLight pointLights[10];
    int numPointLights;
}
ubo;

layout(push_constant) uniform Push
{
    mat4 modelMatrix;
    vec3 color;
}
push;

void main()
{
    vec3 surfaceNormal = normalize(inNormalWorld);

    outColor = vec4(push.color, 1.0f);
}
