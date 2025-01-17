#version 450

layout(location = 0) in vec3 fragPositionWorld;
layout(location = 1) in vec3 fragNormalWorld;

layout(location = 0) out vec4 outColor;

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

void main()
{
    vec3 diffuseLight = ubo.ambientLight.xyz * ubo.ambientLight.w;
    vec3 surfaceNormal = normalize(fragNormalWorld);

    for (int i = 0; i < ubo.numPointLights; i++)
    {
        PointLight light = ubo.pointLights[i];
        vec3 directionToLight = light.position.xyz - fragPositionWorld;

        float attenuation = 1.0f / dot(directionToLight, directionToLight);
        vec3 intensity = light.color.xyz * light.color.w * attenuation;

        float cosAngIncidence = max(dot(surfaceNormal, normalize(directionToLight)), 0.0f);

        diffuseLight += intensity * cosAngIncidence;
    }

    outColor = vec4(diffuseLight * vec3(1.f), 1.0f);
}
