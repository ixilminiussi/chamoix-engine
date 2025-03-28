#ifndef CMX_PHYSICS_UTIL
#define CMX_PHYSICS_UTIL

// cmx
#include "cmx_math.h"

// lib
#include <glm/common.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/vector_float3.hpp>

// std
#include <vector>

namespace cmx
{

inline int findPointFurthestInDir(const std::vector<glm::vec3> &candidates, const glm::vec3 &dir)
{
    int j = 0;
    float maxDst = 0.f;

    for (int i = 1; i < candidates.size(); i++)
    {
        float newDst = glm::dot(candidates[i], dir);
        if (newDst > maxDst)
        {
            newDst = maxDst;
            j = i;
        }
    }

    return j;
}

inline int findPointFurthestFromLine(const std::vector<glm::vec3> &candidates, const glm::vec3 &a, const glm::vec3 &b)
{
    int j = 0;
    float maxDst = 0.f;

    for (int i = 1; i < candidates.size(); i++)
    {
        const glm::vec3 &candidate = candidates[i];
        if (candidate == a || candidate == b)
        {
            continue;
        }

        float newDst = glm::length2(candidate - a) + glm::length2(candidate - b);

        if (newDst > maxDst)
        {
            newDst = maxDst;
            j = i;
        }
    }

    return j;
}

inline int findPointFurthestFromFace(const std::vector<glm::vec3> &candidates, const glm::vec3 &a, const glm::vec3 &b,
                                     const glm::vec3 &c)
{
    int j = 0;
    float maxDst = 0.f;

    for (int i = 1; i < candidates.size(); i++)
    {
        const glm::vec3 &candidate = candidates[i];

        if (candidate == a || candidate == b || candidate == c)
        {
            continue;
        }

        float newDst = std::abs(getTripleProduct(a, b, c, candidate));

        if (newDst > maxDst)
        {
            newDst = maxDst;
            j = i;
        }
    }

    return j;
}

inline glm::vec2 signedVolume1D(const glm::vec3 &s1, const glm::vec3 &s2)
{
    glm::vec3 ab = s1 - s2;
    glm::vec3 p0 = s1 + ab * glm::dot(ab, -s1) / glm::length2(ab);

    int idx = 0;
    float mu_max = 0;
    for (int i = 0; i < 3; i++)
    {
        float mu = s2[i] - s1[i];
        if (mu * mu > mu_max * mu_max)
        {
            mu_max = mu;
            idx = i;
        }
    }

    const float a = s1[idx];
    const float b = s2[idx];
    const float p = p0[idx];

    const float C1 = p - a;
    const float C2 = b - p;

    if ((p > a && p < b) || (p > b && p < a))
    {
        glm::vec2 lambdas;
        lambdas[0] = C2 / mu_max;
        lambdas[1] = C1 / mu_max;
        return lambdas;
    }

    if ((a <= b && p <= a) || (a >= b && p >= a))
    {
        return {1.0f, 0.0f};
    }

    return {0.0f, 1.0f};
}

inline glm::vec3 signedVolume2D(const glm::vec3 &s1, const glm::vec3 &s2, const glm::vec3 &s3)
{
    glm::vec3 normal = glm::cross((s2 - s1), (s3 - s1));
    glm::vec3 p0 = normal * glm::dot(s1, normal) / glm::length2(normal);

    // Find the axis with the greatest projected area
    int idx = 0;
    float area_max = 0;
    for (int i = 0; i < 3; i++)
    {
        int j = (i + 1) % 3;
        int k = (i + 2) % 3;

        glm::vec2 a = glm::vec2(s1[j], s1[k]);
        glm::vec2 b = glm::vec2(s2[j], s2[k]);
        glm::vec2 c = glm::vec2(s3[j], s3[k]);
        glm::vec2 ab = b - a;
        glm::vec2 ac = c - a;

        float area = ab.x * ac.y - ab.y * ac.x;
        if (area * area > area_max * area_max)
        {
            idx = i;
            area_max = area;
        }
    }

    // Project onto the appropriate axis
    int x = (idx + 1) % 3;
    int y = (idx + 2) % 3;
    std::array<glm::vec2, 3> s;
    s[0] = glm::vec2(s1[x], s1[y]);
    s[1] = glm::vec2(s2[x], s2[y]);
    s[2] = glm::vec2(s3[x], s3[y]);
    glm::vec2 p = glm::vec2(p0[x], p0[y]);

    // Get the sub-areas of the triangles formed from the projected origin and the edges
    glm::vec3 areas;
    for (int i = 0; i < 3; i++)
    {
        int j = (i + 1) % 3;
        int k = (i + 2) % 3;

        glm::vec2 a = p;
        glm::vec2 b = s[j];
        glm::vec2 c = s[k];
        glm::vec2 ab = b - a;
        glm::vec2 ac = c - a;

        areas[i] = ab.x * ac.y - ab.y * ac.x;
    }

    // If the projected origin is inside the triangle, then return the barycentric points
    if (compareSigns(area_max, areas[0]) > 0 && compareSigns(area_max, areas[1]) > 0 &&
        compareSigns(area_max, areas[2]) > 0)
    {
        glm::vec3 lambdas = areas / area_max;
        return lambdas;
    }

    float dist = 1e10;
    glm::vec3 lambdas = glm::vec3(1, 0, 0);
    for (int i = 0; i < 3; i++)
    {
        int k = (i + 1) % 3;
        int l = (i + 2) % 3;

        std::array<glm::vec3, 3> edgesPts;
        edgesPts[0] = s1;
        edgesPts[1] = s2;
        edgesPts[2] = s3;

        glm::vec2 lambdaEdge = signedVolume1D(edgesPts[k], edgesPts[l]);
        glm::vec3 pt = edgesPts[k] * lambdaEdge[0] + edgesPts[l] * lambdaEdge[1];
        if (glm::length2(pt) < dist)
        {
            dist = glm::length2(pt);
            lambdas[i] = 0;
            lambdas[k] = lambdaEdge[0];
            lambdas[l] = lambdaEdge[1];
        }
    }

    return lambdas;
}

inline glm::vec4 signedVolume3D(const glm::vec3 &s1, const glm::vec3 &s2, const glm::vec3 &s3, const glm::vec3 &s4)
{
    glm::mat4 M;
    M[0] = glm::vec4(s1.x, s2.x, s3.x, s4.x);
    M[1] = glm::vec4(s1.y, s2.y, s3.y, s4.y);
    M[2] = glm::vec4(s1.z, s2.z, s3.z, s4.z);
    M[3] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    M = glm::transpose(M);

    glm::vec4 C4;
    C4[0] = cofactor(M, 3, 0);
    C4[1] = cofactor(M, 3, 1);
    C4[2] = cofactor(M, 3, 2);
    C4[3] = cofactor(M, 3, 3);

    const float detM = C4[0] + C4[1] + C4[2] + C4[3];

    // If the barycentric coordinates put the origin inside the simplex, then return them
    if (compareSigns(detM, C4[0]) > 0 && compareSigns(detM, C4[1]) > 0 && compareSigns(detM, C4[2]) > 0 &&
        compareSigns(detM, C4[3]) > 0)
    {
        glm::vec4 lambdas = C4 * (1.0f / detM);
        return lambdas;
    }

    // If we get here, then we need to project the origin onto the faces and determine the closest one
    glm::vec4 lambdas;
    float dist = 1e10;
    for (int i = 0; i < 4; i++)
    {
        int j = (i + 1) % 4;
        int k = (i + 2) % 4;

        std::array<glm::vec3, 4> facePts = {s1, s2, s3, s4};

        glm::vec3 lambdasFace = signedVolume2D(facePts[i], facePts[j], facePts[k]);
        glm::vec3 pt = facePts[i] * lambdasFace[0] + facePts[j] * lambdasFace[1] + facePts[k] * lambdasFace[2];
        if (glm::length2(pt) < dist)
        {
            dist = glm::length2(pt);
            lambdas[i] = lambdasFace[0];
            lambdas[j] = lambdasFace[1];
            lambdas[k] = lambdasFace[2];
        }
    }

    return lambdas;
}

glm::vec3 BarycentricCoordinates(glm::vec3 s1, glm::vec3 s2, glm::vec3 s3, const glm::vec3 &pt)
{
    s1 = s1 - pt;
    s2 = s2 - pt;
    s3 = s3 - pt;

    glm::vec3 normal = glm::cross((s2 - s1), (s3 - s1));
    glm::vec3 p0 = normal * glm::dot(s1, normal) / glm::length2(normal);

    // Find the axis with the greatest projected area
    int idx = 0;
    float area_max = 0;
    for (int i = 0; i < 3; i++)
    {
        int j = (i + 1) % 3;
        int k = (i + 2) % 3;

        glm::vec2 a = glm::vec2(s1[j], s1[k]);
        glm::vec2 b = glm::vec2(s2[j], s2[k]);
        glm::vec2 c = glm::vec2(s3[j], s3[k]);
        glm::vec2 ab = b - a;
        glm::vec2 ac = c - a;

        float area = ab.x * ac.y - ab.y * ac.x;
        if (area * area > area_max * area_max)
        {
            idx = i;
            area_max = area;
        }
    }

    // Project onto the appropriate axis
    int x = (idx + 1) % 3;
    int y = (idx + 2) % 3;
    std::array<glm::vec2, 3> s;
    s[0] = glm::vec2(s1[x], s1[y]);
    s[1] = glm::vec2(s2[x], s2[y]);
    s[2] = glm::vec2(s3[x], s3[y]);
    glm::vec2 p = glm::vec2(p0[x], p0[y]);

    // Get the sub-areas of the triangles formed from the projected origin and the edges
    glm::vec3 areas;
    for (int i = 0; i < 3; i++)
    {
        int j = (i + 1) % 3;
        int k = (i + 2) % 3;

        glm::vec2 a = p;
        glm::vec2 b = s[j];
        glm::vec2 c = s[k];
        glm::vec2 ab = b - a;
        glm::vec2 ac = c - a;

        areas[i] = ab.x * ac.y - ab.y * ac.x;
    }

    glm::vec3 lambdas = areas / area_max;
    if (glm::any(glm::isnan(lambdas)) || glm::any(glm::isinf(lambdas)))
    {
        lambdas = glm::vec3(1, 0, 0);
    }
    return lambdas;
}

} // namespace cmx

#endif
