#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;
layout(location = 4) in vec3 tangent;

layout(location = 0) out vec3 outPositionWorld;
layout(location = 1) out vec3 outColor;
layout(location = 2) out vec3 outNormalWorld;
layout(location = 3) out vec2 outUV;
layout(location = 4) out vec4 outPositionLightSpace;

layout(location = 5) out vec3 outDarkColor;
layout(location = 6) out vec3 outLightColor;

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

// we default to this one such that stretches don't affect us.
vec2 getWorldSpaceUV()
{
    vec3 up = abs(outNormalWorld.z) > 0.999 ? vec3(1, 0, 0) : vec3(0, 0, 1);
    vec3 tangent = normalize(cross(outNormalWorld, up));
    vec3 bitangent = cross(outNormalWorld, tangent);

    return (vec2(dot(outPositionWorld, tangent), dot(outPositionWorld, bitangent)));
}

void main()
{
    bool worldSpaceUV = push.normalMatrix[3][3] != 0.f;
    outDarkColor.x = push.modelMatrix[0][3];
    outDarkColor.y = push.modelMatrix[1][3];
    outDarkColor.z = push.modelMatrix[2][3];
    outLightColor.x = push.normalMatrix[0][3];
    outLightColor.y = push.normalMatrix[1][3];
    outLightColor.z = push.normalMatrix[2][3];

    mat4 modelMatrix = push.modelMatrix;
    modelMatrix[0][3] = 0;
    modelMatrix[1][3] = 0;
    modelMatrix[2][3] = 0;
    modelMatrix[3][3] = 1;

    vec4 worldPosition = modelMatrix * vec4(position, 1.0f);
    gl_Position = ubo.projectionMatrix * ubo.viewMatrix * worldPosition;

    outNormalWorld = normalize(mat3(push.normalMatrix) * normal);
    outPositionWorld = worldPosition.xyz;
    outColor = color;
    outPositionLightSpace = ubo.sun.projectionMatrix * ubo.sun.viewMatrix * worldPosition;

    outUV = worldSpaceUV ? getWorldSpaceUV() : uv;
}
