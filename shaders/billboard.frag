#version 450

layout(location = 0) in vec2 inOffset;
layout(location = 1) in vec2 inUV;

layout(set = 1, binding = 0) uniform sampler2D textureSampler;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;

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

layout(push_constant) uniform Push
{
    vec4 position;
    vec4 color;
    vec2 scale;
}
push;

void main()
{
    outColor = vec4(push.color.rgb, 1.0) * texture(textureSampler, inUV);
    outNormal = vec4(0.0);
}
