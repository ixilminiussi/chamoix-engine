#version 450

layout(location = 0) in vec3 fragPositionWorld;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec3 fragNormalWorld;
layout(location = 3) in vec2 fragUV;
layout(location = 4) in vec4 fragPositionLightSpace;

layout(set = 1, binding = 0) uniform sampler3D ditheringSampler;
layout(set = 2, binding = 0) uniform sampler2D textureSampler;
layout(set = 3, binding = 0) uniform sampler2D shadowMapSampler;

layout(location = 0) out vec4 outColor;

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

const float PI = 3.1314f;

vec2 getUVFrequency()
{
    vec2 dx = dFdx(fragUV);
    vec2 dy = dFdy(fragUV);

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
    float spacing = uvFrequency.y; // smaller of the two
    spacing *= exp2(scale);
    spacing /= brightness;

    float logCurve = log2(spacing);
    float roundedLogCurve = floor(logCurve);

    float subLayer = logCurve - roundedLogCurve;
    subLayer = 1.f - subLayer;

    return vec2(exp2(roundedLogCurve), subLayer);
}

float getShadowFactor()
{
    vec3 projCoords = fragPositionLightSpace.xyz / fragPositionLightSpace.w;
    vec2 lightUVCoords = 0.5 * projCoords.xy + 0.5;

    float depth = texture(shadowMapSampler, lightUVCoords).r;

    if (projCoords.z - depth < 0.025)
    {
        return 1.0f;
    }
    return 0.2f;
}

vec3 getDiffuseLight()
{
    // ambient light
    vec3 diffuseLight = ubo.ambientLight.xyz * ubo.ambientLight.w;
    vec3 surfaceNormal = normalize(fragNormalWorld);

    // directional light
    if (ubo.sun.color.w > 0)
    {
        float cosAngIncidence = max(dot(surfaceNormal, -ubo.sun.direction.xyz), 0.0f);
        diffuseLight += ubo.sun.color.xyz * ubo.sun.color.w * min(getShadowFactor(), cosAngIncidence);
    }

    return diffuseLight;
}

vec4 getColor(vec3 alignedColor, vec3 maskColor, vec2 UVOffset)
{
    const vec3 masked = alignedColor * maskColor;

    const float brightness = dot(masked, maskColor);

    const vec2 ditheringLevel = propperDitheringScale(brightness);
    const vec4 ditheringSample =
        texture(ditheringSampler, vec3((fragUV + UVOffset) / ditheringLevel.x, ditheringLevel.y));

    return brightness * push.normalMatrix[3][1] < 1 - ditheringSample.x ? vec4(0.f, 0.f, 0.f, 1.f)
                                                                        : vec4(maskColor, 1.f);
}

vec2 safeUV(vec2 uv)
{
    return clamp(uv, vec2(0.0), vec2(1.0));
}

float gaussian(float x, float sigma)
{
    return exp(-(x * x) / (2.0 * sigma * sigma));
}

vec4 blurSample()
{
    vec4 color = texture(textureSampler, safeUV(fragUV));
    float weights = 1.f;
    float sigma = 200.f;

    float totalDistance = 0.015;
    float totalSteps = 80.f;

    float distanceStep = totalDistance / totalSteps;
    float angleStep = 1.618;

    float distance = distanceStep;

    for (int i = 0; i < 80; i++)
    {
        vec2 offset = distance * vec2(cos(angleStep * i), sin(angleStep * i));
        vec2 uv = safeUV(fragUV + offset);
        color += texture(textureSampler, uv);
        weights += gaussian(distance, sigma);

        distance += distanceStep;
    }

    color /= weights;
    return color;
}
void main()
{
    const vec3 diffuseLight = getDiffuseLight();
    const vec3 textureColor = texture(textureSampler, fragUV).xyz;

    const vec3 combinedColor = diffuseLight * textureColor;

    const float m = push.normalMatrix[3][2];
    const float PI_third = 2.0f * PI / 3.f;

    outColor = getColor(combinedColor, vec3(1.f, 0.f, 0.f), m * vec2(cos(PI_third), sin(PI_third)));
    outColor += getColor(combinedColor, vec3(0.f, 1.f, 0.f), m * vec2(cos(2 * PI_third), sin(2 * PI_third)));
    outColor += getColor(combinedColor, vec3(0.f, 0.f, 1.f), m * vec2(1.0f, 0.f));

    outColor.a = 1.0f;
}
