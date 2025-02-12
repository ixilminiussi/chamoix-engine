#version 450

layout(location = 0) in vec2 fragUV;

layout(set = 1, binding = 0) uniform sampler2D textureSampler;

layout(location = 0) out vec4 outColor;

void main()
{
    if (abs(fragUV.x - 0.5f) < 0.001 && abs(fragUV.y - 0.5f) < 0.001)
    {
        outColor = vec4(1.f);
    }
    else
    {
        discard;
    }
    // outColor = texture(textureSampler, fragUV);
}
