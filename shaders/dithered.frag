#version 450

layout(location = 0) in vec3 fragPositionWorld;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec3 fragNormalWorld;
layout(location = 3) in vec2 fragUV;

layout(set = 1, binding = 0) uniform sampler2D textureSampler;
layout(set = 2, binding = 0) uniform sampler3D ditheringSampler;

layout(location = 0) out vec4 outColor;

struct DirectionalLight
{
    vec4 direction;
    vec4 color;
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
    mat4 normalMatrix; // + color on last column
}
push;

float getLuminance(vec3 color)
{
    return 0.2126f * color.r + 0.7152f * color.g + 0.0722f * color.b;
}

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
    const float scale = 5.5f;
    const int dotsPerSide = 8;
    const float invZRes = 1.f / (64.f - 16.f);

    vec2 uvFrequency = getUVFrequency();
    float spacing = uvFrequency.y; // smaller of the two
    spacing *= exp2(scale);
    spacing /= brightness;
    spacing *= dotsPerSide * 0.125;

    float logCurve = log2(spacing);
    float roundedLogCurve = floor(logCurve);

    float subLayer = logCurve - roundedLogCurve;
    subLayer = 1.f - subLayer;

    return vec2(exp2(roundedLogCurve), subLayer);
}

void main()
{
    // ambient light
    vec3 diffuseLight = ubo.ambientLight.xyz * ubo.ambientLight.w;
    vec3 surfaceNormal = normalize(fragNormalWorld);

    // point lights
    for (int i = 0; i < ubo.numPointLights; i++)
    {
        PointLight light = ubo.pointLights[i];
        vec3 directionToLight = light.position.xyz - fragPositionWorld;

        float attenuation = 1.0f / dot(directionToLight, directionToLight);
        vec3 intensity = light.color.xyz * light.color.w * attenuation;

        float cosAngIncidence = max(dot(surfaceNormal, normalize(directionToLight)), 0.0f);

        diffuseLight += intensity * cosAngIncidence;
    }

    // directional light
    if (ubo.sun.color.w > 0)
    {
        float cosAngIncidence = max(dot(surfaceNormal, -ubo.sun.direction.xyz), 0.0f);
        diffuseLight += ubo.sun.color.xyz * ubo.sun.color.w * cosAngIncidence;
    }

    float brightness = getLuminance(diffuseLight);
    brightness /= 2;

    vec2 ditheringLevel = propperDitheringScale(brightness);
    vec4 ditheringSample = texture(ditheringSampler, vec3(fragUV / ditheringLevel.x, ditheringLevel.y));

    outColor = brightness < 1 - ditheringSample.x ? vec4(0.004f, 0.125f, 0.306f, 1) : vec4(0.965f, 0.863f, 0.675f, 1.f);
}
