#version 450

layout(location = 0) in vec2 inUV;

layout(set = 0, binding = 0) uniform sampler2D sColor;
layout(set = 1, binding = 0) uniform sampler2D sNormal;
layout(set = 2, binding = 0) uniform sampler2D sDepth;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push
{
    int mode;
}
push;

void main()
{
    switch (push.mode)
    {
    case 0:
        outColor = texture(sColor, inUV);
        break;
    case 1:
        outColor = texture(sNormal, inUV);
        break;
    case 2:
        outColor = vec4(texture(sDepth, inUV).r);
        break;
    }

    outColor.a = 1.0;
}
