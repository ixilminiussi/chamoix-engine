#version 450

layout(location = 0) in vec2 inUV;

layout(set = 0, binding = 0) uniform sampler2D sColor;
layout(set = 1, binding = 0) uniform sampler2D sNormal;
layout(set = 2, binding = 0) uniform sampler2D sDepth;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(texture(sColor, inUV).xyz, 1.0);
}
