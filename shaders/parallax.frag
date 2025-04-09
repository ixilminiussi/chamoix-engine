#version 450

layout(location = 0) in vec3 inPositionWorld;
layout(location = 1) in vec3 inNormalWorld;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inUV;
layout(location = 4) in vec4 inPositionLightSpace;

layout(location = 5) in vec3 inSunDirectionTangent;
layout(location = 6) in vec3 inCameraPositionTangent;
layout(location = 7) in vec3 inPositionTangent;

layout(set = 1, binding = 0) uniform sampler2D sColor;
layout(set = 2, binding = 0) uniform sampler2D sNormalHeight;
layout(set = 3, binding = 0) uniform sampler2D sShadowMap;

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

vec3 getDiffuseLight(vec3 surfaceNormal)
{
    // ambient light
    vec3 diffuseLight = ubo.ambientLight.xyz * ubo.ambientLight.w;
    surfaceNormal = normalize(surfaceNormal);

    // directional light
    if (ubo.sun.color.w > 0)
    {
        const float cosAngIncidence = max(dot(surfaceNormal, -inSunDirectionTangent), 0.0f);
        const vec3 projCoords = inPositionLightSpace.xyz / inPositionLightSpace.w;
        diffuseLight += ubo.sun.color.xyz * ubo.sun.color.w * min(getPCFShadow(projCoords), cosAngIncidence);
    }

    return diffuseLight;
}

vec2 parallaxMapping(vec2 uv, vec3 viewDir)
{
    float height = 1.f - texture(sNormalHeight, uv).a;
    vec2 offset = viewDir.xy * (height * .01 + 0.0025) / viewDir.z;

    return uv - offset;
}

void main()
{
    vec2 uv = parallaxMapping(inUV, normalize(inCameraPositionTangent - inPositionTangent));

    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
    {
        discard;
    }

    vec3 normalTangent = texture(sNormalHeight, uv).xyz;
    normalTangent = vec3(normalTangent.x, normalTangent.y, normalTangent.z);
    normalTangent = normalTangent * 2.0f - 1.0f;
    normalTangent = normalize(normalTangent);

    vec3 diffuseLight = getDiffuseLight(normalTangent);

    outColor = vec4(diffuseLight, 1.0f) * texture(sColor, uv);

    outColor.a = 1.0f;
}
