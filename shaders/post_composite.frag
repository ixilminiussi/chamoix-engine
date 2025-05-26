#version 450

layout(location = 0) in vec2 inUV;

layout(set = 0, binding = 0) uniform sampler2D sAlbedo;
layout(set = 1, binding = 0) uniform sampler2D sNormal;
layout(set = 2, binding = 0) uniform sampler2D sShadow;
layout(set = 3, binding = 0) uniform sampler2D sDepth;
layout(set = 4, binding = 0) uniform sampler2D sSSAO;

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

layout(set = 5, binding = 0) uniform GlobalUbo
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
    int ssao;
}
push;

float linearizedDepth(float depth, float near, float far)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));
}

float normalizedDepth(float depth, float near, float far)
{
    return (linearizedDepth(depth, near, far) - near) / (far - near);
}

void main()
{
    vec4 shadow = texture(sShadow, inUV);
    vec4 albedo = texture(sAlbedo, inUV);

    vec3 ambient = ubo.ambientLight.xyz * ubo.ambientLight.w;

    if (push.ssao == 1)
    {
        float ssao = texture(sSSAO, inUV).r;
        ambient *= ssao;
    }

    shadow += vec4(ambient, 1.0);

    outColor = albedo * shadow;

    outColor.a = 1.0;
}
