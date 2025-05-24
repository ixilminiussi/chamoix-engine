#version 450

layout(location = 0) in vec2 inUV;

layout(set = 0, binding = 0) uniform sampler2D sAlbedo;
layout(set = 1, binding = 0) uniform sampler2D sNormal;
layout(set = 2, binding = 0) uniform sampler2D sShadow;
layout(set = 3, binding = 0) uniform sampler2D sDepth;
layout(set = 4, binding = 0) uniform sampler2D sNoise;

layout(location = 0) out float outColor;

layout(push_constant) uniform Push
{
    mat4 projection;
    mat4 view;
}
push;

const vec3 SSAO_DIRECTIONS[16] = {{0.35, 0.20, 0.85},  {-0.30, -0.20, 0.90}, {0.15, -0.35, 0.92}, {-0.10, 0.45, 0.88},
                                  {0.50, -0.10, 0.75}, {-0.45, 0.15, 0.70},  {0.20, 0.60, 0.65},  {-0.60, -0.25, 0.60},
                                  {0.65, 0.40, 0.55},  {-0.70, 0.30, 0.50},  {0.25, -0.70, 0.45}, {-0.75, -0.35, 0.40},
                                  {0.80, 0.10, 0.35},  {-0.20, 0.75, 0.30},  {0.10, -0.80, 0.25}, {-0.85, 0.00, 0.20}};

const float SSAO_RADIUS = 0.2;

void main()
{
    float spl_centerDepth = texture(sDepth, inUV).r;

    if (spl_centerDepth == 1.0)
    {
        discard;
    }

    vec4 c_centerPos = vec4(inUV * 2.0 - 1.0, spl_centerDepth, 1.0);
    vec4 v_centerPos = inverse(push.projection) * c_centerPos;
    v_centerPos /= v_centerPos.w;

    vec3 w_centerNormal = normalize(texture(sNormal, inUV).rgb * 2.0 - 1.0);
    vec3 v_centerNormal = mat3(push.view) * w_centerNormal;

    vec3 randomVec = texture(sNoise, inUV).xyz * 2.0 - 1.0;

    vec3 v_tangent = normalize(randomVec - v_centerNormal * dot(randomVec, v_centerNormal));
    vec3 v_bitangent = cross(v_tangent, v_centerNormal);
    mat3 TBN = mat3(v_tangent, v_bitangent, v_centerNormal);

    float occlusion = 0.0f;

    const float bias = 0.025f;
    for (int i = 0; i < 16; i++)
    {
        vec4 v_offsetPos = v_centerPos + (vec4(TBN * SSAO_DIRECTIONS[i].xyz, 1.0) * SSAO_RADIUS);
        v_offsetPos.w = 1.0;

        vec4 c_offsetPos = push.projection * v_offsetPos;
        c_offsetPos /= c_offsetPos.w;

        float spl_offsetDepth = texture(sDepth, c_offsetPos.xy * 0.5 + 0.5).r;
        vec4 v_truePos = inverse(push.projection) * vec4(c_offsetPos.xy, spl_offsetDepth, 1.0);
        v_truePos /= v_truePos.w;

        float d = abs(v_offsetPos.z - v_centerPos.z);
        float rangeCheck = 1.0 - smoothstep(0.0, SSAO_RADIUS, d);
        occlusion += (v_offsetPos.z >= v_truePos.z + bias ? 1.0f : 0.0f) * rangeCheck;
    }
    occlusion = 1.0 - (occlusion / 16.);

    outColor = occlusion;
}
