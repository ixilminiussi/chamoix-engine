#version 450

layout(location = 0) in vec3 inPositionWorld;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormalWorld;
layout(location = 3) in vec2 inUV;
layout(location = 4) in vec4 inPositionLightSpace;

layout(set = 1, binding = 0) uniform sampler3D ditheringSampler;
layout(set = 2, binding = 0) uniform sampler2D textureSampler;
layout(set = 3, binding = 0) uniform sampler2D shadowMapSampler;

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

const float PI = 3.1314;
const float two_PI = 6.2831;

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

// returns power of 2 for UV scale on x, and the z UV to sample
// from our 3D dithering texture on y
vec2 propperDitheringScale(float brightness)
{
    const float scale = push.normalMatrix[3][0];

    // use the UV frequency to keep all UVs within power of 2
    // of one another
    vec2 uvFrequency = getUVFrequency();
    float spacing = uvFrequency.x * .25 + uvFrequency.y * .75;
    spacing *= exp2(scale);

    // brighter areas means smaller dots/UV frequency
    spacing /= brightness;

    // round down to the nearest power of 2 for the overall
    // UV multiplication
    float logCurve = log2(spacing);
    float roundedLogCurve = floor(logCurve);

    // find the difference between the target UV frequency
    // and the nearest power of 2 to find the correct z
    // to transition within our 3d dithering texture
    float subLayer = logCurve - roundedLogCurve;
    subLayer = 1.0 - subLayer;

    return vec2(exp2(roundedLogCurve), subLayer);
}

float getShadowFactor(vec2 inUV, vec3 projCoords)
{
    float depth = texture(shadowMapSampler, inUV).r;

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
    const vec2 texelSize = 1.0 / textureSize(shadowMapSampler, 0) * 1.5;
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

vec4 getColor(vec3 alignedColor, vec3 maskColor, vec2 UVOffset)
{
    const vec3 masked = alignedColor * maskColor;

    const float brightness = dot(masked, maskColor);

    const vec2 ditheringLevel = propperDitheringScale(brightness);
    const vec4 ditheringSample =
        texture(ditheringSampler, vec3((inUV + UVOffset) / ditheringLevel.x, ditheringLevel.y));

    return brightness * push.normalMatrix[3][1] < 1 - ditheringSample.x ? vec4(0.0, 0.0, 0.0, 1.0)
                                                                        : vec4(maskColor, 1.0);
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
    vec4 color = texture(textureSampler, safeUV(inUV));
    float weights = 1.0;
    float sigma = 200.0;

    float totalDistance = 0.015;
    float totalSteps = 80.0;

    float distanceStep = totalDistance / totalSteps;
    float angleStep = 1.618;

    float distance = distanceStep;

    for (int i = 0; i < 80; i++)
    {
        vec2 offset = distance * vec2(cos(angleStep * i), sin(angleStep * i));
        vec2 uv = safeUV(inUV + offset);
        color += texture(textureSampler, uv);
        weights += gaussian(distance, sigma);

        distance += distanceStep;
    }

    color /= weights;
    return color;
}

vec2 flattenUVs()
{
    // Estimate screen-space gradients of the UVs
    vec2 dx = dFdx(inUV);
    vec2 dy = dFdy(inUV);

    // Compute the length (magnitude) of the change in each direction
    float lenX = length(dx);
    float lenY = length(dy);

    float scaleLimit = 4.; // max scaling correction
    lenX = clamp(lenX, 1.0 / scaleLimit, scaleLimit);
    lenY = clamp(lenY, 1.0 / scaleLimit, scaleLimit);

    // Normalize by the average (or max) to make both axes scale evenly
    float avgLen = (lenX + lenY) * 0.5;

    return (inUV - 0.5) / vec2(lenX, lenY) * avgLen + 0.5;
}

void main()
{
    const vec3 diffuseLight = getDiffuseLight();
    const vec3 textureColor = texture(textureSampler, inUV).xyz;

    const vec3 combinedColor = diffuseLight * textureColor;

    const float m = push.normalMatrix[3][2];
    const float PI_third = 2.0 * PI / 3.0;

    outColor = getColor(combinedColor, vec3(1.0, 0.0, 0.0), m * vec2(cos(PI_third), sin(PI_third)));
    outColor += getColor(combinedColor, vec3(0.0, 1.0, 0.0), m * vec2(cos(2 * PI_third), sin(2 * PI_third)));
    outColor += getColor(combinedColor, vec3(0.0, 0.0, 1.0), m * vec2(1.0, 0.0));

    outColor.a = 1.0;
    outNormal = vec4(inNormalWorld, 1.0);
}
