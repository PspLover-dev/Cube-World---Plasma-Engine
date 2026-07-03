#include "plasma/Tessellate.hpp"

#include <cmath>
#include <cstdint>

namespace plasma {

namespace {

float cross2D(const Vec2& a, const Vec2& b, const Vec2& c) {
    return (b[0] - a[0]) * (c[1] - a[1]) - (b[1] - a[1]) * (c[0] - a[0]);
}

bool pointInTriangle2D(const Vec2& p, const Vec2& a, const Vec2& b, const Vec2& c) {
    const float c1 = cross2D(a, b, p);
    const float c2 = cross2D(b, c, p);
    const float c3 = cross2D(c, a, p);
    const bool hasNeg = (c1 < 0.f) || (c2 < 0.f) || (c3 < 0.f);
    const bool hasPos = (c1 > 0.f) || (c2 > 0.f) || (c3 > 0.f);
    return !(hasNeg && hasPos);
}

bool isEar(const std::vector<Vec2>& poly, const std::vector<int>& indices, int prev, int curr, int next) {
    const Vec2& a = poly[static_cast<size_t>(indices[static_cast<size_t>(prev)])];
    const Vec2& b = poly[static_cast<size_t>(indices[static_cast<size_t>(curr)])];
    const Vec2& c = poly[static_cast<size_t>(indices[static_cast<size_t>(next)])];
    if (cross2D(a, b, c) <= 0.f) {
        return false;
    }
    for (size_t i = 0; i < indices.size(); ++i) {
        if (static_cast<int>(i) == prev || static_cast<int>(i) == curr || static_cast<int>(i) == next) {
            continue;
        }
        if (pointInTriangle2D(poly[static_cast<size_t>(indices[i])], a, b, c)) {
            return false;
        }
    }
    return true;
}

Vec3 catmullRomPoint(const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& p3, float t) {
    const float t2 = t * t;
    const float t3 = t2 * t;
    Vec3 out{};
    for (int i = 0; i < 3; ++i) {
        out[i] = 0.5f * ((2.f * p1[i]) + (-p0[i] + p2[i]) * t +
                           (2.f * p0[i] - 5.f * p1[i] + 4.f * p2[i] - p3[i]) * t2 +
                           (-p0[i] + 3.f * p1[i] - 3.f * p2[i] + p3[i]) * t3);
    }
    return out;
}

} // namespace

std::vector<uint32_t> Tessellate::triangulatePolygon2D(const std::vector<Vec2>& contour) {
    std::vector<uint32_t> triangles;
    if (contour.size() < 3) {
        return triangles;
    }
    std::vector<int> indices(contour.size());
    for (size_t i = 0; i < contour.size(); ++i) {
        indices[i] = static_cast<int>(i);
    }

    int guard = 0;
    while (indices.size() > 3 && guard++ < 10000) {
        bool clipped = false;
        const int n = static_cast<int>(indices.size());
        for (int i = 0; i < n; ++i) {
            const int prev = (i + n - 1) % n;
            const int next = (i + 1) % n;
            if (!isEar(contour, indices, prev, i, next)) {
                continue;
            }
            triangles.push_back(static_cast<uint32_t>(indices[static_cast<size_t>(prev)]));
            triangles.push_back(static_cast<uint32_t>(indices[static_cast<size_t>(i)]));
            triangles.push_back(static_cast<uint32_t>(indices[static_cast<size_t>(next)]));
            indices.erase(indices.begin() + i);
            clipped = true;
            break;
        }
        if (!clipped) {
            break;
        }
    }
    if (indices.size() == 3) {
        triangles.push_back(static_cast<uint32_t>(indices[0]));
        triangles.push_back(static_cast<uint32_t>(indices[1]));
        triangles.push_back(static_cast<uint32_t>(indices[2]));
    }
    return triangles;
}

std::vector<Vec3> Tessellate::catmullRomSpline(const std::vector<Vec3>& controlPoints, int segmentsPerSpan,
                                               bool closed) {
    std::vector<Vec3> out;
    if (controlPoints.size() < 2) {
        return controlPoints;
    }
    const int n = static_cast<int>(controlPoints.size());
    const int spans = closed ? n : n - 1;
    for (int i = 0; i < spans; ++i) {
        const Vec3& p0 = controlPoints[static_cast<size_t>((i - 1 + n) % n)];
        const Vec3& p1 = controlPoints[static_cast<size_t>(i)];
        const Vec3& p2 = controlPoints[static_cast<size_t>((i + 1) % n)];
        const Vec3& p3 = controlPoints[static_cast<size_t>((i + 2) % n)];
        for (int s = 0; s < segmentsPerSpan; ++s) {
            const float t = static_cast<float>(s) / static_cast<float>(segmentsPerSpan);
            out.push_back(catmullRomPoint(p0, p1, p2, p3, t));
        }
    }
    if (!closed) {
        out.push_back(controlPoints.back());
    }
    return out;
}

} // namespace plasma
