#version 450

layout(location = 0) in vec2 fragOffset;
layout(location = 1) in vec2 fragUV;

layout(set = 1, binding = 0) uniform sampler2D textureSampler;

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
    vec4 position;
    vec4 color;
    vec2 scale;
}
push;

void main()
{
    // float distance = dot(fragOffset, fragOffset);
    // if (distance >= 1.0)
    // {
    //     discard;
    // }

    // float intensity = pow(1.0f - distance, 5);

    // outColor = vec4(push.color.rgb, intensity);
    outColor = vec4(push.color.rgb, 1.0f) * texture(textureSampler, fragUV);
}
