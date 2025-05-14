#version 450

layout(location = 0) in vec2 inUV;

layout(set = 0, binding = 0) uniform sampler2D sAlbedo;
layout(set = 1, binding = 0) uniform sampler2D sNormal;
layout(set = 2, binding = 0) uniform sampler2D sShadow;
layout(set = 3, binding = 0) uniform sampler2D sDepth;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push
{
    int mode;
    float nearPlane;
    float farPlane;
}
push;

float linearizedDepth(float depth, float near, float far)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));
}

float normalizedDepth(float depth, float near, float far)
{
    return (linearizedDepth(depth, near, far) - near) / (far - near);
}

void main()
{
    switch (push.mode)
    {
    case 0:
        vec4 shadow = texture(sShadow, inUV);
        vec4 albedo = texture(sAlbedo, inUV);
        outColor = albedo * shadow;
        break;
    case 1:
        outColor = texture(sAlbedo, inUV);
        break;
    case 2:
        outColor = texture(sShadow, inUV);
        break;
    case 3:
        outColor = (texture(sNormal, inUV) + 1.0) / 2.0;
        break;
    case 4:
        float depth = texture(sDepth, inUV).r;
        depth = normalizedDepth(depth, push.nearPlane, push.farPlane);
        outColor = vec4(depth);
        break;
    }

    outColor.a = 1.0;
}
