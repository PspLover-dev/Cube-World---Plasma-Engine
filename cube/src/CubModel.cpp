#include "cube/Assets.hpp"
#include "cube/BlobDescramble.hpp"

#include <algorithm>
#include <cstring>

namespace cube {

namespace {

bool readLeU32(const uint8_t* p, uint32_t& out) {
    out = static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
          (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24);
    return true;
}

constexpr uint8_t kAirRgb[3] = {0xff, 0xff, 0xff};

bool isAirRgb(const uint8_t* px) {
    // FUN_004e71d0 / DAT_0076b340 — air is white; .cub also uses black for empty cells.
    if (px[0] == kAirRgb[0] && px[1] == kAirRgb[1] && px[2] == kAirRgb[2]) {
        return true;
    }
    return px[0] == 0 && px[1] == 0 && px[2] == 0;
}

} // namespace

bool CubModel::loadFromBlob(const std::vector<uint8_t>& blob, bool descramble) {
    std::vector<uint8_t> copy = blob;
    if (descramble) {
        descrambleBlob(copy);
    }
    if (copy.size() < 12) {
        return false;
    }
    uint32_t w = 0, d = 0, h = 0;
    readLeU32(copy.data(), w);
    readLeU32(copy.data() + 4, d);
    readLeU32(copy.data() + 8, h);
    if (w == 0 || d == 0 || h == 0 || w > 512 || d > 512 || h > 512) {
        return false;
    }
    width = static_cast<int>(w);
    depth = static_cast<int>(d);
    height = static_cast<int>(h);
    const size_t need = static_cast<size_t>(w) * d * h * 3u;
    rgb.assign(need, 0);
    const size_t avail = copy.size() > 12 ? copy.size() - 12 : 0;
    const size_t n = std::min(need, avail);
    if (n > 0) {
        std::memcpy(rgb.data(), copy.data() + 12, n);
    }
    return n > 0;
}

bool CubModel::inBounds(int x, int y, int z) const {
    return x >= 0 && y >= 0 && z >= 0 && x < width && y < depth && z < height;
}

const uint8_t* CubModel::voxelRgb(int x, int y, int z) const {
    if (!inBounds(x, y, z)) {
        return nullptr;
    }
    const size_t off = (static_cast<size_t>(depth) * z + y) * width + x;
    return rgb.data() + off * 3u;
}

bool CubModel::isVoxelEmpty(int x, int y, int z) const {
    const uint8_t* px = voxelRgb(x, y, z);
    if (!px) {
        return true;
    }
    return isAirRgb(px);
}

namespace {

void pushQuad(std::vector<CubMeshVertex>& verts, std::vector<uint32_t>& idx, float x0, float y0,
              float z0, float x1, float y1, float z1, float x2, float y2, float z2, float x3,
              float y3, float z3, float nx, float ny, float nz, uint8_t r, uint8_t g, uint8_t b) {
    const uint32_t base = static_cast<uint32_t>(verts.size());
    verts.push_back({x0, y0, z0, nx, ny, nz, r, g, b});
    verts.push_back({x1, y1, z1, nx, ny, nz, r, g, b});
    verts.push_back({x2, y2, z2, nx, ny, nz, r, g, b});
    verts.push_back({x3, y3, z3, nx, ny, nz, r, g, b});
    idx.insert(idx.end(), {base, base + 1, base + 2, base, base + 2, base + 3});
}

} // namespace

CubMesh CubMeshBuilder::build(const CubModel& model, float voxelSize) {
    CubMesh mesh;
    const float s = voxelSize;
    for (int x = 0; x < model.width; ++x) {
        for (int y = 0; y < model.depth; ++y) {
            for (int z = 0; z < model.height; ++z) {
                if (model.isVoxelEmpty(x, y, z)) {
                    continue;
                }
                const uint8_t* rgb = model.voxelRgb(x, y, z);
                const float fx = x * s, fy = y * s, fz = z * s;
                const float fx1 = fx + s, fy1 = fy + s, fz1 = fz + s;
                auto face = [&](int nx, int ny, int nz, auto quad) {
                    if (model.isVoxelEmpty(x + nx, y + ny, z + nz)) {
                        quad();
                    }
                };
                face(-1, 0, 0, [&] {
                    pushQuad(mesh.vertices, mesh.indices, fx, fy, fz1, fx, fy1, fz1, fx, fy1, fz,
                             fx, fy, fz, -1.f, 0.f, 0.f, rgb[0], rgb[1], rgb[2]);
                });
                face(1, 0, 0, [&] {
                    pushQuad(mesh.vertices, mesh.indices, fx1, fy1, fz1, fx1, fy, fz1, fx1, fy, fz,
                             fx1, fy1, fz, 1.f, 0.f, 0.f, rgb[0], rgb[1], rgb[2]);
                });
                face(0, -1, 0, [&] {
                    pushQuad(mesh.vertices, mesh.indices, fx, fy, fz, fx1, fy, fz, fx1, fy, fz1, fx,
                             fy, fz1, 0.f, -1.f, 0.f, rgb[0], rgb[1], rgb[2]);
                });
                face(0, 1, 0, [&] {
                    pushQuad(mesh.vertices, mesh.indices, fx, fy1, fz, fx, fy1, fz1, fx1, fy1, fz1,
                             fx1, fy1, fz, 0.f, 1.f, 0.f, rgb[0], rgb[1], rgb[2]);
                });
                face(0, 0, -1, [&] {
                    pushQuad(mesh.vertices, mesh.indices, fx, fy1, fz, fx1, fy1, fz, fx1, fy, fz,
                             fx, fy, fz, 0.f, 0.f, -1.f, rgb[0], rgb[1], rgb[2]);
                });
                face(0, 0, 1, [&] {
                    pushQuad(mesh.vertices, mesh.indices, fx, fy, fz1, fx1, fy, fz1, fx1, fy1, fz1,
                             fx, fy1, fz1, 0.f, 0.f, 1.f, rgb[0], rgb[1], rgb[2]);
                });
            }
        }
    }
    return mesh;
}

} // namespace cube
