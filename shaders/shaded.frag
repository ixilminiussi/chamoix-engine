#version 450

layout(location = 0) in vec3 inPositionWorld;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormalWorld;
layout(location = 3) in vec2 inUV;
layout(location = 4) in vec4 inPositionLightSpace;

layout(set = 1, binding = 0) uniform sampler2D sColor;
layout(set = 2, binding = 0) uniform sampler2D sShadowMap;

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
    vec4 cameraPosition;
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

const float PI = 3.1415;
const float two_PI = 6.2831;

float getShadowFactor(vec2 inUV, vec3 projCoords)
{
    float depth = texture(sShadowMap, inUV).r;

    if (projCoords.z - depth < 0.025)
    {
        return 1.0f;
    }
    return 0.f;
}

float getPCFShadow(vec3 projCoords)
{
    const float angle = two_PI / 8.f;
    const vec2 lightUVCoords = 0.5 * projCoords.xy + 0.5;
    const vec2 texelSize = 1.0f / textureSize(sShadowMap, 0) * 1.5f;
    ;

    float combined = getShadowFactor(lightUVCoords, projCoords);

    for (int i = 0; i < 8; i++)
    {
        const vec2 uvOffset = vec2(cos(angle * i), sin(angle * i)) * texelSize.x;
        combined += getShadowFactor(lightUVCoords + uvOffset, projCoords);
    }

    return combined / 9.f;
}

vec3 getDiffuseLight()
{
    // ambient light
    vec3 diffuseLight = ubo.ambientLight.xyz * ubo.ambientLight.w;
    vec3 surfaceNormal = normalize(inNormalWorld);

    // point lights
    for (int i = 0; i < ubo.numPointLights; i++)
    {
        PointLight light = ubo.pointLights[i];
        vec3 directionToLight = light.position.xyz - inPositionWorld;

        float attenuation = 1.0f / dot(directionToLight, directionToLight);
        vec3 intensity = light.color.xyz * light.color.w * attenuation;

        float cosAngIncidence = max(dot(surfaceNormal, -normalize(directionToLight)), 0.0f);

        diffuseLight += intensity * cosAngIncidence;
    }

    // directional light
    if (ubo.sun.color.w > 0)
    {
        const float cosAngIncidence = max(dot(surfaceNormal, -ubo.sun.direction.xyz), 0.0f);
        const vec3 projCoords = inPositionLightSpace.xyz / inPositionLightSpace.w;
        diffuseLight += ubo.sun.color.xyz * ubo.sun.color.w * min(getPCFShadow(projCoords), cosAngIncidence);
    }

    return diffuseLight;
}

void main()
{
    vec3 diffuseLight = getDiffuseLight();

    // if we have push.normalMatrix[3][3] != 100, then we should be using vertex color
    if (push.normalMatrix[3][3] == 100)
    {
        outColor = vec4(diffuseLight * push.normalMatrix[3].xyz * inColor, 1.0f);
    }
    else
    {
        outColor = vec4(diffuseLight * push.normalMatrix[3].xyz, 1.0f) * texture(sColor, inUV);
    }

    outColor.a = 1.0f;
}
