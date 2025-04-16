#version 450

layout(location = 0) in vec2 inUV;

layout(set = 0, binding = 0) uniform sampler2D sColor;
layout(set = 1, binding = 0) uniform sampler2D sNormal;
layout(set = 2, binding = 0) uniform sampler2D sDepth;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push
{
    vec2 colorSettings;
    vec2 normalSettings;
    vec2 depthSettings;
    float nearPlane;
    float farPlane;
}
push;

float linearizeDepth(float depth, float near, float far)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));
}

bool isDepthEdge(vec2 uv, float threshold, float edgeThickness)
{
    const vec2 texelSize = (0.2 * edgeThickness) / textureSize(sDepth, 0);

    float sampleN = linearizeDepth(texture(sDepth, uv + vec2(0.0, texelSize.y)).r, push.nearPlane, push.farPlane);
    float sampleS = linearizeDepth(texture(sDepth, uv + vec2(0.0, -texelSize.y)).r, push.nearPlane, push.farPlane);
    float sampleE = linearizeDepth(texture(sDepth, uv + vec2(texelSize.x, 0.0)).r, push.nearPlane, push.farPlane);
    float sampleW = linearizeDepth(texture(sDepth, uv + vec2(-texelSize.x, 0.0)).r, push.nearPlane, push.farPlane);
    float sampleNE = linearizeDepth(texture(sDepth, uv + texelSize * vec2(1.0, 1.0)).r, push.nearPlane, push.farPlane);
    float sampleNW = linearizeDepth(texture(sDepth, uv + texelSize * vec2(-1.0, 1.0)).r, push.nearPlane, push.farPlane);
    float sampleSE = linearizeDepth(texture(sDepth, uv + texelSize * vec2(1.0, -1.0)).r, push.nearPlane, push.farPlane);
    float sampleSW =
        linearizeDepth(texture(sDepth, uv + texelSize * vec2(-1.0, -1.0)).r, push.nearPlane, push.farPlane);

    float gx = sampleNW - sampleNE + 2.0 * sampleW - 2.0 * sampleE + sampleSW - sampleSE;

    float gy = sampleNW - sampleSW + 2.0 * sampleN - 2.0 * sampleS + sampleNE - sampleSE;

    float edgeFactor = length(vec2(gx, gy));

    return edgeFactor > threshold;
}

bool isNormalEdge(vec2 uv, float threshold, float edgeThickness)
{
    const vec2 texelSize = (0.1 * edgeThickness) / textureSize(sNormal, 0);

    vec4 sampleN = 2.0 * texture(sNormal, uv + vec2(0.0, texelSize.y)) - 1.0;
    vec4 sampleS = 2.0 * texture(sNormal, uv + vec2(0.0, -texelSize.y)) - 1.0;
    vec4 sampleE = 2.0 * texture(sNormal, uv + vec2(texelSize.x, 0.0)) - 1.0;
    vec4 sampleW = 2.0 * texture(sNormal, uv + vec2(-texelSize.x, 0.0)) - 1.0;
    vec4 sampleNE = 2.0 * texture(sNormal, uv + texelSize * vec2(1.0, 1.0)) - 1.0;
    vec4 sampleNW = 2.0 * texture(sNormal, uv + texelSize * vec2(-1.0, 1.0)) - 1.0;
    vec4 sampleSE = 2.0 * texture(sNormal, uv + texelSize * vec2(1.0, -1.0)) - 1.0;
    vec4 sampleSW = 2.0 * texture(sNormal, uv + texelSize * vec2(-1.0, -1.0)) - 1.0;

    float gx = dot(sampleNW.rgb - sampleNE.rgb, sampleNW.rgb - sampleNE.rgb) +
               2.0 * dot(sampleW.rgb - sampleE.rgb, sampleW.rgb - sampleE.rgb) +
               dot(sampleSW.rgb - sampleSE.rgb, sampleSW.rgb - sampleSE.rgb);

    float gy = dot(sampleNW.rgb - sampleSW.rgb, sampleNW.rgb - sampleSW.rgb) +
               2.0 * dot(sampleN.rgb - sampleS.rgb, sampleN.rgb - sampleS.rgb) +
               dot(sampleNE.rgb - sampleSE.rgb, sampleNE.rgb - sampleSE.rgb);

    float edgeFactor = length(vec2(gx, gy));

    return edgeFactor > threshold;
}

void main()
{
    if (push.depthSettings.x > 0. && isDepthEdge(inUV, push.depthSettings.y, push.depthSettings.x))
    {
        outColor = vec4(1.0);
        return;
    }
    if (push.normalSettings.x > 0. && isNormalEdge(inUV, push.normalSettings.y, push.normalSettings.x))
    {
        outColor = vec4(1.0);
        return;
    }

    discard;
}
