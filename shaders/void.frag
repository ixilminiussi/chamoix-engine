#version 450

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
