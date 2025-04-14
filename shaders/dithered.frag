#version 450

layout(location = 0) in vec3 inPositionWorld;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormalWorld;
layout(location = 3) in vec2 inUV;
layout(location = 4) in vec4 inPositionLightSpace;

layout(location = 5) in vec3 inDarkColor;
layout(location = 6) in vec3 inLightColor;

layout(set = 1, binding = 0) uniform sampler3D sDitheringPattern;
layout(set = 2, binding = 0) uniform sampler2D sShadowMap;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;

struct DirectionalLight
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
    vec4 color;
    vec4 direction;
};

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
    vec4 cameraPosition;
    DirectionalLight sun;
    PointLight pointLights[10];
    int numPointLights;
}
ubo;

layout(push_constant) uniform Push
{
    mat4 modelMatrix;
    mat4 normalMatrix;
}
push;

const float two_PI = 6.2831;

float getLuminance(vec3 color)
{
    return 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
}

vec2 getUVFrequency()
{
    vec2 dx = dFdx(inUV);
    vec2 dy = dFdy(inUV);

    mat2 matr = {dx, dy};
    vec4 vectorized = vec4(dx, dy);
    float Q = dot(vectorized, vectorized);
    float R = determinant(matr); // ad-bc
    float discriminantSqr = max(0, Q * Q - 4 * R * R);
    float discriminant = sqrt(discriminantSqr);

    return sqrt(vec2(Q + discriminant, Q - discriminant) / 2);
}

vec2 propperDitheringScale(float brightness)
{
    const float scale = push.normalMatrix[3][0];

    vec2 uvFrequency = getUVFrequency();
    float spacing = (uvFrequency.x * .25 + uvFrequency.y * .75);
    spacing *= exp2(scale);
    spacing /= brightness;

    float logCurve = log2(spacing);
    float roundedLogCurve = floor(logCurve);

    float subLayer = logCurve - roundedLogCurve;
    subLayer = 1.0 - subLayer;

    return vec2(exp2(roundedLogCurve), subLayer);
}

float getShadowFactor(vec2 inUV, vec3 projCoords)
{
    float depth = texture(sShadowMap, inUV).r;

    if (projCoords.z - depth < 0.025)
    {
        return 1.0;
    }
    return 0.2;
}

float getPCFShadow(vec3 projCoords)
{
    const float angle = two_PI / 8.0;
    const vec2 lightUVCoords = 0.5 * projCoords.xy + 0.5;
    const vec2 texelSize = 1.0 / textureSize(sShadowMap, 0) * 1.5;
    ;

    float combined = getShadowFactor(lightUVCoords, projCoords);

    for (int i = 0; i < 8; i++)
    {
        const vec2 uvOffset = vec2(cos(angle * i), sin(angle * i)) * texelSize.x;
        combined += getShadowFactor(lightUVCoords + uvOffset, projCoords);
    }

    return combined / 9.0;
}

vec3 getDiffuseLight()
{
    // ambient light
    vec3 diffuseLight = ubo.ambientLight.xyz * ubo.ambientLight.w;
    vec3 surfaceNormal = normalize(inNormalWorld);

    // directional light
    if (ubo.sun.color.w > 0)
    {
        float cosAngIncidence = max(dot(surfaceNormal, -ubo.sun.direction.xyz), 0.0);
        const vec3 projCoords = inPositionLightSpace.xyz / inPositionLightSpace.w;
        diffuseLight += ubo.sun.color.xyz * ubo.sun.color.w * min(getPCFShadow(projCoords), cosAngIncidence);
    }

    return diffuseLight;
}

void main()
{
    bool dotToggle = ((uint(push.normalMatrix[3][2]) & 1u) == 0u);

    vec3 diffuseLight = getDiffuseLight();

    float brightness = dotToggle ? 1 - getLuminance(diffuseLight) : getLuminance(diffuseLight);
    brightness /= 2;

    vec2 ditheringLevel = propperDitheringScale(brightness);
    vec4 ditheringSample = texture(sDitheringPattern, vec3(inUV / ditheringLevel.x, ditheringLevel.y));

    if (dotToggle)
    {
        outColor = brightness * push.normalMatrix[3][1] < 1 - ditheringSample.x ? vec4(inLightColor, 1.0)
                                                                                : vec4(inDarkColor, 1.0);
    }
    else
    {
        outColor = brightness * push.normalMatrix[3][1] < 1 - ditheringSample.x ? vec4(inDarkColor, 1.0)
                                                                                : vec4(inLightColor, 1.0);
    }

    outColor.a = 1.0;
    outNormal = vec4(inNormalWorld, 1.0);
}
