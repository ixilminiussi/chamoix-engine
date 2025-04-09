#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;
layout(location = 4) in vec3 inTangent;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec3 outPositionWorld;
layout(location = 2) out vec3 outNormalWorld;

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
    vec4 worldPosition = push.modelMatrix * vec4(inPosition, 1.0f);
    gl_Position = ubo.projectionMatrix * ubo.viewMatrix * worldPosition;

    outPositionWorld = worldPosition.xyz;
    outColor = inColor;
}
