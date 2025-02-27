#version 450

layout(location = 0) in vec3 fragPositionWorld;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec3 fragNormalWorld;
layout(location = 3) in vec2 fragUV;

layout(set = 1, binding = 0) uniform sampler2D textureSampler;
layout(set = 2, binding = 0) uniform sampler2D ditheringSampler;

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

float getLuminance(vec4 color)
{
    return 0.2126f * color.r + 0.7152f * color.g + 0.0722f * color.b;
}

float targetScreenSpaceScale = .05f;

float propperDitheringScale()
{
    vec2 dx = dFdx(fragUV);
    vec2 dy = dFdy(fragUV);
    float screenSpaceRateOfChange = length(vec2(length(dx), length(dy)));

    float scale = 1.f;

    while (screenSpaceRateOfChange < targetScreenSpaceScale / 2.f)
    {
        screenSpaceRateOfChange *= 2.f;
        scale *= 2.f;
    }
    while (screenSpaceRateOfChange > targetScreenSpaceScale * 2.f)
    {
        screenSpaceRateOfChange /= 2.f;
        scale /= 2.f;
    }

    return scale;
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

    // if we have push.normalMatrix[3][3] != 100, then we should be using vertex color
    if (push.normalMatrix[3][3] == 100)
    {
        outColor = vec4(diffuseLight * push.normalMatrix[3].xyz * fragColor, 1.0f);
    }
    else
    {
        outColor = vec4(diffuseLight * push.normalMatrix[3].xyz, 1.0f) * texture(textureSampler, fragUV);
    }

    vec4 ditheringSample = texture(ditheringSampler, fragUV * propperDitheringScale());
    if (getLuminance(outColor) < ditheringSample.x)
    {
        outColor = vec4(0, 0, 0, 1);
    }
    else
    {
        outColor = vec4(1, 1, 1, 1);
    }
}
