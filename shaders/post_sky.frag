#version 450

layout(location = 0) in vec2 inUV;
layout(location = 1) in vec3 inRayDir;

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

float airMass(float radians)
{
    if (radians < 0.)
        return 0.;
    return 1 / (cos(radians) + 0.50572 * pow(96.07995 - degrees(radians), -1.6364));
}

void main()
{
    if (texture(sDepth, inUV).r != 1.0)
    {
        discard;
    }

    // NOT IDEAL (but avoids distortions) move it to cube texture
    vec2 ndc = inUV * 2.0 - 1.0;
    vec4 clip = vec4(ndc, -1.0, 1.0);

    vec4 viewDir = inverse(ubo.projectionMatrix) * clip;
    viewDir /= viewDir.w;
    viewDir.w = 0.0;

    vec4 worldDir = inverse(ubo.viewMatrix) * viewDir;
    vec3 rayDir = normalize(worldDir.xyz);

    float cosTheta = dot(rayDir, vec3(0., -1., 0.));
    // below earth, ignore
    if (cosTheta < 0.)
    {
        outColor = vec4(1.0);
        return;
    }

    float sunCosTheta = pow(dot(rayDir, -ubo.sun.direction.xyz), 10);

    outColor.xyz = mix(ubo.sun.color.xyz, vec3(1.0), max(1.0 - cosTheta, sunCosTheta));

    outColor.a = 1.0;
}
