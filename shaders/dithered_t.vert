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

vec2 getWorldSpaceUV()
{
    float scale = push.normalMatrix[2][3];
    float theta = push.normalMatrix[3][3];

    vec2 uvOffset = vec2(push.normalMatrix[0][3], push.normalMatrix[1][3]);

    mat2 rotationMatrix = mat2(cos(theta), -sin(theta), sin(theta), cos(theta));

    vec3 up = abs(outNormalWorld.z) > 0.999 ? vec3(1, 0, 0) : vec3(0, 0, 1);
    vec3 tangent = normalize(cross(outNormalWorld, up));
    vec3 bitangent = cross(outNormalWorld, tangent);

    return rotationMatrix * (vec2(dot(outPositionWorld, tangent), dot(outPositionWorld, bitangent)) / scale + uvOffset);
}

void main()
{
    mat4 modelMatrix = push.modelMatrix;

    vec4 worldPosition = modelMatrix * vec4(position, 1.0f);
    gl_Position = ubo.projectionMatrix * ubo.viewMatrix * worldPosition;

    outNormalWorld = normalize(mat3(push.normalMatrix) * normal);
    outPositionWorld = worldPosition.xyz;
    outColor = color;
    outPositionLightSpace = ubo.sun.projectionMatrix * ubo.sun.viewMatrix * worldPosition;

    if (push.normalMatrix[2][3] == 0.f)
    {
        vec2 uvOffset = vec2(push.normalMatrix[0][3], push.normalMatrix[1][3]);

        vec2 adaptedUV = vec2(uv.x, 1.0 - uv.y);

        outUV = adaptedUV + uvOffset;
    }
    else
    {
        outUV = getWorldSpaceUV();
    }
}
