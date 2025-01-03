#version 450

layout(location = 0) in vec2 fragOffset;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUbo
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
    vec3 lightPosition;
    vec4 lightColor;
    vec4 ambientLight;
}
ubo;

void main()
{
    float distance = dot(fragOffset, fragOffset);
    if (distance >= 1.0)
    {
        discard;
    }
    outColor = vec4(ubo.lightColor.xyz, 1.0f - sqrt(distance, 2));
}
