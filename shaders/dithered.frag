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
    // get partial derivative of UVs in respect to screen space x and y
    vec2 dx = dFdx(inUV);
    vec2 dy = dFdy(inUV);

    // uses eigen value to determine the two principal rates of change
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
    // whether or not we use light dots on dark background, or the reverse
    const bool dotToggle = ((uint(push.normalMatrix[3][2]) & 1u) == 0u);

    const vec3 diffuseLight = getDiffuseLight();

    float brightness = dotToggle ? 1 - getLuminance(diffuseLight) : getLuminance(diffuseLight);
    brightness /= 2;

    const vec2 ditheringLevel = propperDitheringScale(brightness);

    const vec2 uvFrequency = getUVFrequency();
    const vec4 ditheringSample = texture(sDitheringPattern, vec3(flattenUVs() / ditheringLevel.x, ditheringLevel.y));

    // push.normalMatrix[3][1] is our specified threshold
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
