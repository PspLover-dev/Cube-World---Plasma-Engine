#include "cube/WorldMap.hpp"

namespace cube {

WorldMap::WorldMap(const WorldInfo& info, int width, int height)
    : info_(info), width_(width), height_(height), tiles_(static_cast<size_t>(width) * height, 0) {}

void WorldMap::setTile(int x, int y, int spriteId) {
    if (x >= 0 && y >= 0 && x < width_ && y < height_) {
        tiles_[static_cast<size_t>(y) * width_ + static_cast<size_t>(x)] = spriteId;
    }
}

int WorldMap::tile(int x, int y) const {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) {
        return 0;
    }
    return tiles_[static_cast<size_t>(y) * width_ + static_cast<size_t>(x)];
}

} // namespace cube
