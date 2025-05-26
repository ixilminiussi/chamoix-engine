#version 450

layout(location = 0) in vec2 inUV;

layout(set = 1, binding = 0) uniform sampler2D sDepth;

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

void main()
{
    if (texture(sDepth, inUV).r != 1.0)
    {
        discard;
    }
    outColor = vec4(ubo.ambientLight.xyz, 1.0);
}
