#version 450

layout(location = 0) in vec2 inUV;

layout(set = 0, binding = 0) uniform sampler2D sDepth;
layout(set = 1, binding = 0) uniform sampler2D sSSAO;

layout(location = 0) out float outColor;

layout(push_constant) uniform Push
{
    float nearPlane;
    float farPlane;
}
push;

void main()
{
    float ssao = texture(sSSAO, inUV).r;
    float count = 9.0;

    vec2 step = 2.0 / textureSize(sSSAO, 0);
    ssao += texture(sSSAO, vec2(inUV.x - step.x, inUV.y - step.y)).r;
    ssao += texture(sSSAO, vec2(inUV.x - step.x, inUV.y)).r;
    ssao += texture(sSSAO, vec2(inUV.x - step.x, inUV.y + step.y)).r;
    ssao += texture(sSSAO, vec2(inUV.x, inUV.y - step)).r;
    ssao += texture(sSSAO, vec2(inUV.x, inUV.y + step)).r;
    ssao += texture(sSSAO, vec2(inUV.x + step.x, inUV.y - step.y)).r;
    ssao += texture(sSSAO, vec2(inUV.x + step.x, inUV.y)).r;
    ssao += texture(sSSAO, vec2(inUV.x + step.x, inUV.y + step.y)).r;

    ssao /= count;

    outColor = ssao;
}
