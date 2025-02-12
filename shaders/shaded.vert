#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragPositionWorld;
layout(location = 1) out vec3 fragNormalWorld;
layout(location = 2) out vec2 fragUV;
// layout(location = 2) out vec3 fragColor;

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
    vec4 worldPosition = push.modelMatrix * vec4(position, 1.0f);
    gl_Position = ubo.projectionMatrix * ubo.viewMatrix * worldPosition;

    fragNormalWorld = normalize(mat3(push.normalMatrix) * normal);
    fragPositionWorld = worldPosition.xyz;

    vec2 uvOffset = vec2(push.normalMatrix[0][3], push.normalMatrix[1][3]);

    // if we have push.normalMatrix[3][3] != 0, then we should be using tiling uv mapping. otherwise it's regular uv
    if (push.normalMatrix[2][3] == 0)
    {
        fragUV = vec2(uv.x, 1.0 - uv.y) + uvOffset;
    }
    else
    {
        vec2 adaptedUV = vec2(uv.x, 1.0 - uv.y);

        float scale = push.normalMatrix[2][3];
        float theta = push.normalMatrix[3][3];

        mat2 rotationMatrix = mat2(cos(theta), -sin(theta), sin(theta), cos(theta));

        vec3 up = abs(fragNormalWorld.z) > 0.999 ? vec3(1, 0, 0) : vec3(0, 0, 1);
        vec3 tangent = normalize(cross(fragNormalWorld, up));
        vec3 bitangent = cross(fragNormalWorld, tangent);

        fragUV = rotationMatrix *
                 (vec2(dot(fragPositionWorld, tangent), dot(fragPositionWorld, bitangent)) / scale + uvOffset);
    }
}
