#version 450

layout(location = 0) in float customFragDepth;

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
}
