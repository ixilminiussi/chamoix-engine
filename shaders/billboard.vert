#version 450

const vec2 OFFSETS[6] =
    vec2[](vec2(-1.0, -1.0), vec2(-1.0, 1.0), vec2(1.0, -1.0), vec2(1.0, -1.0), vec2(-1.0, 1.0), vec2(1.0, 1.0));

layout(location = 0) out vec2 fragOffset;

layout(set = 0, binding = 0) uniform GlobalUbo
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
    vec3 lightPosition;
    vec4 lightColor;
    vec4 ambientLight;
}
ubo;

const float LIGHT_RADIUS = 0.1;

void main()
{
    fragOffset = OFFSETS[gl_VertexIndex];

    vec4 lightInCameraSpace = ubo.viewMatrix * vec4(ubo.lightPosition, 1.0);
    vec4 positionInCameraSpace = lightInCameraSpace + LIGHT_RADIUS * vec4(fragOffset, 0.0f, 0.0f);
    gl_Position = ubo.projectionMatrix * positionInCameraSpace;
}
