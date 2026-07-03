#pragma once

#include "plasma/Math.hpp"

#include <cstdint>
#include <vector>

namespace plasma {

/// Ear-clipping polygon tessellation (replaces GLU for SmoothMeshShape).
class Tessellate {
public:
    static std::vector<uint32_t> triangulatePolygon2D(const std::vector<Vec2>& contour);
    static std::vector<Vec3> catmullRomSpline(const std::vector<Vec3>& controlPoints, int segmentsPerSpan,
                                             bool closed);
};

} // namespace plasma
