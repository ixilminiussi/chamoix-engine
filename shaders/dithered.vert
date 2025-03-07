#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragPositionWorld;
layout(location = 1) out vec3 fragColor;
layout(location = 2) out vec3 fragNormalWorld;
layout(location = 3) out vec2 fragUV;

layout(location = 4) out vec3 fragDarkColor;
layout(location = 5) out vec3 fragLightColor;

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
}
ubo;

layout(push_constant) uniform Push
{
    mat4 modelMatrix;
    mat4 normalMatrix;
}
push;

vec2 getWorldView()
{
    vec3 up = abs(fragNormalWorld.z) > 0.999 ? vec3(1, 0, 0) : vec3(0, 0, 1);
    vec3 tangent = normalize(cross(fragNormalWorld, up));
    vec3 bitangent = cross(fragNormalWorld, tangent);

    return vec2(dot(fragPositionWorld, tangent), dot(fragPositionWorld, bitangent));
}

void main()
{
    fragDarkColor.x = push.modelMatrix[0][3];
    fragDarkColor.y = push.modelMatrix[1][3];
    fragDarkColor.z = push.modelMatrix[2][3];
    fragLightColor.x = push.normalMatrix[0][3];
    fragLightColor.y = push.normalMatrix[1][3];
    fragLightColor.z = push.normalMatrix[2][3];

    mat4 modelMatrix = push.modelMatrix;
    modelMatrix[0][3] = 0;
    modelMatrix[1][3] = 0;
    modelMatrix[2][3] = 0;
    modelMatrix[3][3] = 1;

    vec4 worldPosition = modelMatrix * vec4(position, 1.0f);
    gl_Position = ubo.projectionMatrix * ubo.viewMatrix * worldPosition;

    fragNormalWorld = normalize(mat3(push.normalMatrix) * normal);
    fragPositionWorld = worldPosition.xyz;
    fragColor = color;

    if ((uint(push.normalMatrix[3][2]) & 2u) == 0u)
    {
        fragUV = uv;
    }
    else
    {
        fragUV = getWorldView();
    }
}
