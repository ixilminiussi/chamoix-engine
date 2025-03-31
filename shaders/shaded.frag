#version 450

layout(location = 0) in vec3 fragPositionWorld;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec3 fragNormalWorld;
layout(location = 3) in vec2 fragUV;
layout(location = 4) in vec4 fragPositionLightSpace;

layout(set = 1, binding = 0) uniform sampler2D textureSampler;
layout(set = 2, binding = 0) uniform sampler2D shadowMapSampler;

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
    mat4 normalMatrix; // + color on last column
}
push;

float calcShadowFactor()
{
    vec3 projCoords = fragPositionLightSpace.xyz / fragPositionLightSpace.w;
    vec2 lightUVCoords = 0.5 * projCoords.xy + 0.5;

    float depth = texture(shadowMapSampler, lightUVCoords).r;

    if (projCoords.z - depth < 0.025)
    {
        return 1.0f;
    }
    return 0.5f;
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

        float cosAngIncidence = max(dot(surfaceNormal, -normalize(directionToLight)), 0.0f);

        diffuseLight += intensity * cosAngIncidence;
    }

    // directional light
    if (ubo.sun.color.w > 0)
    {
        float depthOnShadowSampler = texture(shadowMapSampler, fragPositionLightSpace.xy).r;
        float depthOnLightSpace = fragPositionLightSpace.z;

        float cosAngIncidence = max(dot(surfaceNormal, -ubo.sun.direction.xyz), 0.0f);
        diffuseLight += ubo.sun.color.xyz * ubo.sun.color.w * min(calcShadowFactor(), cosAngIncidence);
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

    outColor.a = 1.0f;
}
