#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;
layout(location = 4) in vec3 inTangent;

layout(set = 0, binding = 0) uniform ShadowUbo
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
}
ubo;

layout(push_constant) uniform Push
{
    mat4 modelMatrix;
    mat4 normalMatrix;
}
push;

void main()
{
    vec4 worldPosition = push.modelMatrix * vec4(inPosition, 1.0f);
    gl_Position = ubo.projectionMatrix * ubo.viewMatrix * worldPosition;
}
