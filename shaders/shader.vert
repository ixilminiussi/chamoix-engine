#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform Push
{
    mat4 transform;
    mat4 normalMatrix;
}
push;

const vec3 DIRECTION_TO_LIGHT = normalize(vec3(1., -3., -1.));
const float GLOBAL_ILLUMINATION = .02f;

void main()
{
    gl_Position = push.transform * vec4(position, 1.0f);

    vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * normal);

    float lightIntensity = max(dot(normalWorldSpace, DIRECTION_TO_LIGHT), 0) + GLOBAL_ILLUMINATION;

    fragColor = lightIntensity * color;
}
