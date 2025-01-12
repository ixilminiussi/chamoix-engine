#version 450

const vec2 OFFSETS[6] =
    vec2[](vec2(-1.0, -1.0), vec2(-1.0, 1.0), vec2(1.0, -1.0), vec2(1.0, -1.0), vec2(-1.0, 1.0), vec2(1.0, 1.0));

layout(location = 0) out vec2 fragOffset;

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
    vec4 position;
    vec4 color;
    vec2 scale;
}
push;

void main()
{
    fragOffset = OFFSETS[gl_VertexIndex];

    vec3 centerInWorldSpace = push.position.xyz;

    vec3 right = vec3(ubo.viewMatrix[0][0], ubo.viewMatrix[1][0], ubo.viewMatrix[2][0]);
    vec3 up = vec3(ubo.viewMatrix[0][1], ubo.viewMatrix[1][1], ubo.viewMatrix[2][1]);

    vec3 offsetInWorldSpace = fragOffset.x * right * push.scale.x + fragOffset.y * up * push.scale.y;
    vec3 vertexPosInWorldSpace = centerInWorldSpace + offsetInWorldSpace;

    gl_Position = ubo.projectionMatrix * ubo.viewMatrix * vec4(vertexPosInWorldSpace, 1.0);
}
