#version 450

layout(location = 0) in vec2 inUV;

layout(set = 1, binding = 0) uniform sampler2D textureSampler;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;

void main()
{
    if (abs(inUV.x - 0.5) < 0.001 && abs(inUV.y - 0.5) < 0.001)
    {
        outColor = vec4(1.0);
        outNormal = vec4(0.0);
    }
    else
    {
        discard;
    }
}
